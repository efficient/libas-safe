mod dl;
mod dlfcn;
mod got;
mod mman;

#[link(name = "lib")]
extern "C" {
	fn get() -> bool;
	fn set(_: bool);
}

fn main() {
	use got::GOT;

	assert!(! unsafe {
		get()
	});
	unsafe {
		set(true);
	}
	assert!(unsafe {
		get()
	});

	let base = GOT::new().unwrap();
	let replace = base.clone().unwrap();
	unsafe {
		replace.install();
	}

	assert!(! unsafe {
		get()
	});
	unsafe {
		set(true);
	}
	assert!(unsafe {
		get()
	});
	unsafe {
		set(false);
	}
	assert!(! unsafe {
		get()
	});

	unsafe {
		base.install();
	}

	assert!(unsafe {
		get()
	});
}
