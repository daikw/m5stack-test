examples/%:
	mkdir -p $@
	touch $@/main.cpp

delete/%:
	rm -rf examples/$(subst delete/,,$@)

switch/%:
	unlink src/main.cpp
	ln -s ../examples/$(subst switch/,,$@)/main.cpp src/main.cpp
