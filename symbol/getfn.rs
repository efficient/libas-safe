extern crate gotcha;
use libc;
use std;

use gotcha::Group;
use gotcha::group_lookup_symbol_fn;
use gotcha::group_thread_set;
use gotcha::shared_hook;
use libc::free;
use std::cell::Cell;
use std::os::raw::c_void;
use std::process::abort;
use std::ptr::null_mut;

thread_local! {
	static EXPECTED: Cell<bool> = Cell::default();
}

fn main() {
	let group = Group::new().unwrap();
	let free0: unsafe extern fn(*mut c_void) = unsafe {
		group_lookup_symbol_fn!(Group::SHARED, free)
	}.unwrap();
	let free1: unsafe extern fn(*mut c_void) = unsafe {
		group_lookup_symbol_fn!(group, free)
	}.unwrap();
	assert!(free0 != free);
	assert!(free0 != free1);
	shared_hook(callback);

	unsafe {
		free(null_mut());
		free0(null_mut());
		free1(null_mut());
	}

	group_thread_set!(group);
	EXPECTED.with(|expected| expected.set(true));
	unsafe {
		free(null_mut());
	}
	if EXPECTED.with(|expected| expected.get()) {
		abort();
	}
	unsafe {
		free0(null_mut());
		free1(null_mut());
	}

	EXPECTED.with(|expected| expected.set(true));
	group_thread_set!(Group::SHARED);
}

extern fn callback() {
	EXPECTED.with(|expected| {
		assert!(expected.get());
		expected.set(false);
	});
}
