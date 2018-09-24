mod dl;
mod dlfcn;
mod got;

fn main() {
	use got::GOT;

	let shadow = GOT::default().clone().unwrap();
	unsafe {
		shadow.install();
	}
}
