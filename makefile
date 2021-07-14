prefix = "/usr/local"

xdg_runtime_dir: main.o
	cc $? -o $@

main.o: main.c
	cc -c $? -o $@

install: xdg_runtime_dir xdg_runtime_dir.sh
	cp xdg_runtime_dir "$(prefix)/bin/"
	cp xdg_runtime_dir.sh /etc/profile.d/
