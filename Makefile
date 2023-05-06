examples/%:
	mkdir -p $@
	touch $@/main.cpp

delete/%:
	rm -rf examples/$(subst delete/,,$@)

switch/%:
	unlink src
	ln -s examples/$(subst switch/,,$@) src
