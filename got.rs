use dl::Addr;
use dl::Handle;
use dl::Namespace;
use std::borrow::Cow;
use std::collections::HashMap;
use std::collections::VecDeque;
use std::ffi::CStr;
use std::ffi::CString;
use std::fmt::Debug;
use std::fmt::Error as FmtError;
use std::fmt::Formatter;
use std::iter::FromIterator;
use std::ops::Deref;
use std::result::Result as StdResult;
use std::sync::Mutex;

type Result<T> = StdResult<T, Error>;

struct Table {
	symbols: Cow<'static, [(usize, &'static CStr, &'static CStr)]>,
	entries: Box<[Addr]>,
}

pub struct GOT {
	namespace: Namespace,
	handles: VecDeque<Handle>,
	table: Table,
}

impl GOT {
	pub fn new() -> Result<Self> {
		use dl::Flags;
		use dl::dlmopen;
		use dl::dlsym;

		let template = Singleton::handle();
		let mut namespace = template.namespace();
		let mut handles = HashMap::new();
		let symbols = &*template.original.symbols;
		let mut entries = template.original.entries.clone();
		for (index, filename, symbol) in &*symbols {
			let handle = handles.entry(filename).or_insert_with(||
				dlmopen(&mut namespace, filename, Flags::NOW)
			).clone().map_err(|or| Error {
				result: or,
				namespace,
				handles: distill(handles.drain()),
			})?;
			match dlsym(handle, symbol) {
				Ok(ay) => entries[*index] = ay,
				Err(or) => eprintln!("{:?}", or),
			}
		}

		let symbols = Cow::Borrowed(symbols);
		let handles = distill(handles);
		Ok(Self {
			namespace,
			handles,
			table: Table {
				symbols,
				entries,
			}
		})
	}

	pub unsafe fn uninstall() -> Option<Self> {
		Singleton::handle().installed.lock().unwrap().take()
	}

	pub unsafe fn install(self) -> Option<Self> {
		use dl::global_offset_table_mut;
		use mman::PROT_READ;
		use mman::PROT_WRITE;
		use mman::mprotect;
		use std::io::Error;
		use std::mem::size_of_val;

		static mut FIRST: bool = true;

		let mut installed = Singleton::handle().installed.lock().unwrap();
		let old = installed.take();
		let got = global_offset_table_mut();
		if FIRST {
			let got = got.as_ptr() as usize;
			let page = got & !0xfffusize;
			let size = got - page + size_of_val(&got);
			if mprotect(page as _, size, (PROT_READ | PROT_WRITE) as _) != 0 {
				panic!(Error::last_os_error());
			}
			FIRST = false;
		}
		got.copy_from_slice(&self.table.entries);
		*installed = Some(self);
		old
	}
}

struct Singleton {
	recycle: Mutex<Vec<Namespace>>,
	original: Table,
	installed: Mutex<Option<GOT>>,
}

impl Singleton {
	fn handle() -> &'static Self {
		use dl::dladdr;
		use dl::global_offset_table;
		use std::sync::ONCE_INIT;
		use std::sync::Once;

		static INIT: Once = ONCE_INIT;
		static mut SINGLETON: Option<Singleton> = None;

		INIT.call_once(|| {
			let mut symbols = Vec::new();
			let entries = Vec::from(global_offset_table()).into_boxed_slice();
			for (index, entry) in entries.iter().enumerate() {
				if let Some(info) = dladdr(*entry) {
					if let Some(symbol) = info.symbol {
						let library = info.filename.to_bytes_with_nul()
							.rsplit(|it| *it == b'/').next().unwrap();
						if ! library.starts_with(b"ld") {
							symbols.push((index, info.filename, symbol));
						}
					}
				}
			}

			let symbols = Cow::Owned(symbols);
			unsafe {
				SINGLETON = Some(Self {
					recycle: Mutex::new(Vec::new()),
					original: Table {
						symbols,
						entries,
					},
					installed: Mutex::new(None),
				});
			}
		});

		unsafe {
			SINGLETON.as_ref()
		}.unwrap()
	}

	fn namespace(&self) -> Namespace {
		self.recycle.lock().unwrap().pop().unwrap_or(Namespace::NEW)
	}
}

impl Drop for GOT {
	fn drop(&mut self) {
		use dl::dlclose;

		for handle in self.handles.drain(..) {
			dlclose(handle).unwrap();
		}
		Singleton::handle().recycle.lock().unwrap().push(self.namespace)
	}
}

pub struct Error {
	result: CString,
	namespace: Namespace,
	handles: VecDeque<Handle>,
}

impl Debug for Error {
	fn fmt(&self, f: &mut Formatter) -> StdResult<(), FmtError> {
		write!(f, "{:?}", &self.result)
	}
}

impl Deref for Error {
	type Target = CString;

	fn deref(&self) -> &Self::Target {
		&self.result
	}
}

impl Drop for Error {
	fn drop(&mut self) {
		use dl::dlclose;

		for handle in self.handles.drain(..) {
			dlclose(handle).unwrap();
		}
		Singleton::handle().recycle.lock().unwrap().push(self.namespace)
	}
}

fn distill<K, V, E: Debug, I: IntoIterator<Item = (K, StdResult<V, E>)>, C: FromIterator<V>>(map: I) -> C {
	map.into_iter().map(|(_, val)| val.unwrap()).collect()
}
