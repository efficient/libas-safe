#![feature(test)]
#![feature(rustc_private)]
extern crate libc;
extern crate test;
use test::Bencher;

#[bench]
fn spawn(lo: &mut Bencher) {
	use std::thread::spawn;

	lo.iter(|| spawn(|| ()));
}

#[bench]
fn pthread_create(lo: &mut Bencher) {
	use libc::pthread_create;
	use libc::pthread_join;
	use std::ffi::c_void;
	use std::ptr;

	lo.iter(|| {
		let mut tid = 0;
		unsafe {
			pthread_create(&mut tid, ptr::null(), nop, ptr::null_mut());
			pthread_join(tid, ptr::null_mut());
		}
	});

	extern fn nop(_: *mut c_void) -> *mut c_void { ptr::null_mut() }
}
