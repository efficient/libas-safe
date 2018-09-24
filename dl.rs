use dlfcn::LM_ID_BASE;
use dlfcn::LM_ID_NEWLM;
use dlfcn::RTLD_LAZY;
use dlfcn::RTLD_NOW;
use dlfcn::Lmid_t;
use std::ffi::CStr;
use std::ffi::CString;
use std::ops::Deref;
use std::os::raw::c_uint;
use std::os::raw::c_void;
use std::result::Result as StdResult;

pub type Result<T> = StdResult<T, CString>;

pub fn dladdr(addr: Addr) -> Option<Info> {
	use dlfcn::Dl_info;
	use dlfcn::dladdr;
	use std::ptr::null;
	use std::ptr::null_mut;

	impl From<Dl_info> for Info {
		fn from(info: Dl_info) -> Self {
			Self {
				filename: unsafe {
					CStr::from_ptr(info.dli_fname)
				},
				base: Addr (info.dli_fbase),
				symbol: if info.dli_sname.is_null() {
					None
				} else {
					Some(unsafe {
						CStr::from_ptr(info.dli_sname)
					})
				},
				address: Addr (info.dli_saddr),
			}
		}
	}

	let mut info = Dl_info {
		dli_fname: null(),
		dli_fbase: null_mut(),
		dli_sname: null(),
		dli_saddr: null_mut(),
	};
	let Addr (addr) = addr;
	if unsafe {
		dladdr(addr, &mut info)
	} == 0 {
		None?;
	}

	Some(info.into())
}

pub fn dlclose(handle: Handle) -> Result<()> {
	use dlfcn::dlclose;

	let Handle (handle) = handle;
	if unsafe {
		dlclose(handle)
	} != 0 {
		Err(dlerror().unwrap().to_owned())?;
	}

	Ok(())
}

fn dlerror() -> Option<&'static CStr> {
	use dlfcn::dlerror;

	let error = unsafe {
		dlerror()
	};
	if error.is_null() {
		None?;
	}

	Some(unsafe {
		CStr::from_ptr(error)
	})
}

pub fn dlmopen(namespace: &mut Namespace, filename: &CStr, flags: Flags) -> Result<Handle> {
	use dlfcn::RTLD_DI_LMID;
	use dlfcn::dlinfo;
	use dlfcn::dlmopen;

	let Namespace (namespace) = namespace;
	let Flags (flags) = flags;
	let handle = unsafe {
		dlmopen(*namespace, filename.as_ptr(), flags as _)
	};
	if handle.is_null() || unsafe {
		dlinfo(handle, RTLD_DI_LMID as _, namespace as *mut _ as _)
	} != 0 {
		Err(dlerror().unwrap().to_owned())?;
	}

	Ok(Handle (handle))
}

pub fn dlsym(handle: Handle, symbol: &CStr) -> Result<Addr> {
	use dlfcn::dlsym;

	let Handle (handle) = handle;
	dlerror();

	let addr = unsafe {
		dlsym(handle, symbol.as_ptr())
	};
	if let Some(error) = dlerror() {
		Err(error.to_owned())?;
	}

	Ok(Addr (addr))
}

pub fn global_offset_table() -> &'static [Addr] {
	unsafe {
		global_offset_table_mut()
	}
}

pub unsafe fn global_offset_table_mut() -> &'static mut [Addr] {
	use std::mem::size_of;
	use std::slice;

	extern "C" {
		static mut _GLOBAL_OFFSET_TABLE_: [Addr; 0];
		static data_start: [Addr; 0];
	}

	let got_start = _GLOBAL_OFFSET_TABLE_.as_mut_ptr();
	let got_end = data_start.as_ptr();
	let got_len = (got_end as usize - got_start as usize) / size_of::<Addr>();
	slice::from_raw_parts_mut(got_start, got_len)
}

#[derive(Clone, Copy)]
#[repr(transparent)]
pub struct Addr (pub *const c_void);

impl Deref for Addr {
	type Target = *const c_void;

	fn deref(&self) -> &Self::Target {
		let Addr (addr) = self;
		addr
	}
}

#[derive(Clone, Copy)]
pub struct Flags (c_uint);

#[allow(dead_code)]
impl Flags {
	pub const LAZY: Self = Flags (RTLD_LAZY);
	pub const NOW: Self = Flags (RTLD_NOW);

	pub fn global(&mut self) -> &mut Self {
		use dlfcn::RTLD_GLOBAL;

		{
			let Flags (flags) = self;
			*flags |= RTLD_GLOBAL;
		}

		self
	}

	pub fn local(&mut self) -> &mut Self {
		use dlfcn::RTLD_LOCAL;

		{
			let Flags (flags) = self;
			*flags |= RTLD_LOCAL;
		}

		self
	}

	pub fn nodelete(&mut self) -> &mut Self {
		use dlfcn::RTLD_NODELETE;

		{
			let Flags (flags) = self;
			*flags |= RTLD_NODELETE;
		}

		self
	}

	pub fn noload(&mut self) -> &mut Self {
		use dlfcn::RTLD_NOLOAD;

		{
			let Flags (flags) = self;
			*flags |= RTLD_NOLOAD;
		}

		self
	}

	pub fn deepbind(&mut self) -> &mut Self {
		use dlfcn::RTLD_DEEPBIND;

		{
			let Flags (flags) = self;
			*flags |= RTLD_DEEPBIND;
		}

		self
	}
}

#[derive(Clone, Copy)]
pub struct Handle (*mut c_void);

#[derive(Clone, Copy)]
pub struct Info {
	pub filename: &'static CStr,
	pub base: Addr,
	pub symbol: Option<&'static CStr>,
	pub address: Addr,
}

#[derive(Clone, Copy, Eq, PartialEq)]
pub struct Namespace (Lmid_t);

#[allow(dead_code)]
impl Namespace {
	pub const BASE: Self = Namespace (LM_ID_BASE as _);
	pub const NEW: Self = Namespace (LM_ID_NEWLM as _);
}
