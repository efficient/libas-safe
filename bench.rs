mod dl;
mod dlfcn;
mod got;
mod mman;

fn main() {
	use got::GOT;

	let shadow = GOT::default().clone().unwrap();
	unsafe {
		shadow.install();
	}
}
