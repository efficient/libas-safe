#[allow(dead_code)]
mod prctl;

use lazy_static::lazy_static;
use libc::puts;
use prctl::ARCH_GET_FS;
use std::ffi::CString;
use std::mem::MaybeUninit;
use std::os::raw::c_uint;
extern {
	fn _dl_allocate_tls_init(_: *mut TcbT);
	fn arch_prctl(_: c_uint, _: *mut *mut TcbT);
}

lazy_static! {
	static ref GLOBAL: Ure<'static> = "GLOBAL".into();
}
thread_local! {
	static LOCAL: Ure<'static> = "LOCAL".into();
}

fn main() {
	println!("{:p}", &*GLOBAL);
	LOCAL.with(|local| println!("{:p}", local));

	unsafe {
		let mut tcb = MaybeUninit::uninit();
		arch_prctl(ARCH_GET_FS, tcb.as_mut_ptr());
		_dl_allocate_tls_init(tcb.assume_init());
	}

	LOCAL.with(|local| println!("{:p}", local));
}

struct Ure<'a> (&'a str);

impl<'a> From<&'a str> for Ure<'a> {
	fn from(name: &'a str) -> Self {
		println!("Ure::from({})", name);
		Self (name)
	}
}

impl Drop for Ure<'_> {
	fn drop(&mut self) {
		let Self (name) = self;
		let name = CString::new(format!("Ure::drop({})", name)).unwrap();
		unsafe {
			puts(name.into_raw());
		}
	}
}

enum TcbT {}
