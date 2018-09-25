use dl::Addr;
use dl::Handle;
use dl::Namespace;
use dl::Result as DlResult;
use std::borrow::Cow;
use std::collections::HashMap;
use std::ffi::CStr;
use std::io::Result;
use std::sync::MutexGuard;

pub struct GOT {
	namespace: Namespace,
	handles: HashMap<&'static CStr, DlResult<Handle>>,
	entries: Cow<'static, [Addr]>,
}

impl GOT {
	fn installed() -> MutexGuard<'static, Option<GOT>> {
		use std::sync::Mutex;

		static mut INSTALLED: Option<Mutex<Option<GOT>>> = None;

		unsafe {
			INSTALLED.get_or_insert_with(|| Mutex::new(None))
		}.lock().unwrap_or_else(|or| or.into_inner())
	}

	fn recycle() -> MutexGuard<'static, Vec<Namespace>> {
		use std::sync::Mutex;

		static mut RECYCLE: Option<Mutex<Vec<Namespace>>> = None;

		unsafe {
			RECYCLE.get_or_insert_with(|| Mutex::new(Vec::new()))
		}.lock().unwrap_or_else(|or| or.into_inner())
	}

	pub fn new() -> Result<Self> {
		use dl::global_offset_table;
		use mman::PROT_READ;
		use mman::PROT_WRITE;
		use mman::mprotect;
		use std::io::Error;
		use std::mem::size_of_val;
		use std::sync::ONCE_INIT;
		use std::sync::Once;

		static INIT: Once = ONCE_INIT;
		static mut ORIG: Option<Box<[Addr]>> = None;

		let mut error = None;
		INIT.call_once(|| {
			let got = global_offset_table();
			let page = got.as_ptr() as usize & !0xfffusize;
			if unsafe {
				mprotect(page as _, got.as_ptr() as usize - page + size_of_val(got), (PROT_READ | PROT_WRITE) as _)
			} != 0 {
				error = Some(Error::last_os_error());
			}
			unsafe {
				ORIG = Some(got.to_owned().into_boxed_slice());
			}
		});
		if let Some(error) = error {
			Err(error)?;
		}

		Ok(Self {
			namespace: Namespace::BASE,
			handles: HashMap::new(),
			entries: Cow::Borrowed(unsafe {
				ORIG.as_ref()
			}.unwrap()),
		})
	}

	pub fn clone(&self) -> DlResult<Self> {
		use dl::Flags;
		use dl::dladdr;
		use dl::dlmopen;
		use dl::dlsym;

		let mut namespace = Self::recycle().pop().unwrap_or(Namespace::NEW);
		let mut handles = HashMap::new();
		let mut entries = self.entries.clone();

		for entry in entries.to_mut() {
			if let Some(info) = dladdr(*entry) {
				let library = info.filename.to_bytes_with_nul().rsplit(|it| *it == b'/').next().unwrap();
				if library.starts_with(b"ld") {
					continue;
				}

				if let Some(symbol) = info.symbol {
					let handle = handles
						.entry(info.filename)
						.or_insert_with(|| dlmopen(&mut namespace, info.filename, Flags::NOW))
						.clone()?;
					match dlsym(handle, symbol) {
						Ok(ay) => *entry = ay,
						Err(or) => eprintln!("{:?}", or),
					}
				}
			}
		}

		Ok(Self {
			namespace,
			handles,
			entries,
		})
	}

	pub unsafe fn install(self) -> bool {
		use dl::global_offset_table_mut;

		let mut installed = None;
		if let Cow::Owned(entries) = &self.entries {
			installed = Some(Self::installed());
			global_offset_table_mut().copy_from_slice(entries);
		}

		if let Some(mut installed) = installed {
			*installed = Some(self);
			true
		} else {
			false
		}
	}
}

impl Drop for GOT {
	fn drop(&mut self) {
		use dl::dlclose;

		for (_, handle) in self.handles.drain() {
			dlclose(handle.unwrap()).unwrap();
		}
		if self.namespace != Namespace::BASE {
			Self::recycle().push(self.namespace);
		}
	}
}
