mod dl;
mod dlfcn;
mod got;
mod mman;

fn main() {
	use got::GOT;

	let shadow = GOT::new().unwrap().clone().unwrap();
	unsafe {
		shadow.install();
	}
}
