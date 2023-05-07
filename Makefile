examples/%:
	mkdir -p $@
	touch $@/main.cpp
	unlink src
	ln -s examples/$(subst switch/,,$@) src

delete/%:
	rm -rf examples/$(subst delete/,,$@)

switch/%:
	unlink src
	ln -s examples/$(subst switch/,,$@) src
