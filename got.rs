use dl::Addr;
use dl::Handle;
use dl::Namespace;
use dl::Result;
use std::borrow::Cow;
use std::collections::HashMap;
use std::ffi::CStr;
use std::sync::MutexGuard;

pub struct GOT {
	namespace: Namespace,
	handles: HashMap<&'static CStr, Result<Handle>>,
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

	pub fn clone(&self) -> Result<Self> {
		use dl::Flags;
		use dl::dladdr;
		use dl::dlmopen;
		use dl::dlsym;

		let mut namespace = Self::recycle().pop().unwrap_or(Namespace::NEW);
		let mut handles = HashMap::new();
		let mut entries = self.entries.clone();

		for entry in entries.to_mut() {
			if let Some(info) = dladdr(*entry) {
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

impl Default for GOT {
	fn default() -> Self {
		use dl::global_offset_table;
		use std::sync::ONCE_INIT;
		use std::sync::Once;

		static INIT: Once = ONCE_INIT;
		static mut ORIG: Option<Box<[Addr]>> = None;

		INIT.call_once(|| unsafe {
			ORIG = Some(global_offset_table().to_owned().into_boxed_slice());
		});
		Self {
			namespace: Namespace::BASE,
			handles: HashMap::new(),
			entries: Cow::Borrowed(unsafe {
				ORIG.as_ref()
			}.unwrap()),
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
