use dl::Addr;
use dl::Handle;
use dl::Namespace;
use dl::Result;
use std::borrow::Cow;
use std::collections::HashMap;
use std::collections::HashSet;
use std::collections::VecDeque;
use std::ffi::CStr;
use std::sync::Mutex;

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
			).clone().map_err(|or|
				drop_handles(unwrap_values(handles.drain()), namespace).err().unwrap_or(or)
			)?;
			match dlsym(handle, symbol) {
				Ok(ay) => entries[*index] = ay,
				Err(or) => eprintln!("{:?}", or),
			}
		}

		let symbols = Cow::Borrowed(symbols);
		let handles = unwrap_values(handles).collect();
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
			let blacklist: [&[_]; 17] = [
				b"__libc_calloc\0",
				b"__libc_free\0",
				b"__libc_malloc\0",
				b"__libc_pvalloc\0",
				b"__libc_realloc\0",
				b"__libc_valloc\0",
				b"aligned_alloc\0",
				b"calloc\0",
				b"cfree\0",
				b"free\0",
				b"malloc\0",
				b"malloc_usable_size\0",
				b"memalign\0",
				b"posix_memalign\0",
				b"pvalloc\0",
				b"realloc\0",
				b"valloc\0",
			];
			let blacklist: HashSet<_> = blacklist.into_iter().map(|fun|
				CStr::from_bytes_with_nul(fun).unwrap()
			).collect();

			let mut symbols = Vec::new();
			let entries = Vec::from(global_offset_table()).into_boxed_slice();
			for (index, entry) in entries.iter().enumerate() {
				if let Some(info) = dladdr(*entry) {
					if let Some(symbol) = info.symbol {
						let library = info.filename.to_bytes_with_nul()
							.rsplit(|it| *it == b'/').next().unwrap();
						if ! library.starts_with(b"ld") && ! blacklist.contains(symbol) {
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
		drop_handles(self.handles.drain(..), self.namespace).unwrap();
	}
}

fn unwrap_values<K, V>(map: impl IntoIterator<Item = (K, impl IntoIterator<Item = V>)>) -> impl Iterator<Item = V> {
	map.into_iter().flat_map(|(_, val)| val)
}

fn drop_handles<T: Iterator<Item = Handle>>(handles: T, namespace: Namespace) -> Result<()> {
	use dl::dlclose;

	for handle in handles {
		dlclose(handle)?;
	}
	Singleton::handle().recycle.lock().unwrap().push(namespace);

	Ok(())
}
