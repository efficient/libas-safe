use lazy_static::lazy_static;

lazy_static! {
	static ref GLOBAL: Ure<'static> = "GLOBAL".into();
}
thread_local! {
	static LOCAL: Ure<'static> = "LOCAL".into();
}

fn main() {
	println!("{:p}", &*GLOBAL);
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
		eprintln!("Ure::drop({})", name);
	}
}
