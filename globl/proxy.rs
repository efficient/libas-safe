#![crate_type = "lib"]

pub fn proxy<P: Copy + 'static, R: 'static>(lmid: usize, fun: fn(P) -> R, arg: P) -> Option<R> {
	use std::cell::Cell;

	extern "C" {
		fn proxy(_: usize, _: unsafe extern "C" fn(), _: usize);
	}

	extern "C" fn helper() {
		FUN.with(|cell| cell.take()).unwrap()();
	}

	thread_local! {
		static FUN: Cell<Option<&'static mut dyn FnMut()>> = Cell::default();
	}

	let mut ret = None;
	let mut fun = || drop(ret.replace(fun(arg)));
	let fun: &mut dyn FnMut() = &mut fun;
	let fun = unsafe {
		relax_mut(fun)
	};
	FUN.with(move |cell| cell.replace(Some(fun)));
	unsafe {
		proxy(lmid, helper, 0);
	}

	ret
}

unsafe fn relax_mut<'a, T: ?Sized>(ptr: *mut T) -> &'a mut T {
	&mut *ptr
}
