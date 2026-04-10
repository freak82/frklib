
reldeb:
	cmake --build ./build/ --verbose --config RelWithDebInfo

debug:
	cmake --build ./build/ --verbose --config Debug

release:
	cmake --build ./build/ --verbose --config Release

clean:
	rm -rf ./build/app
	rm -rf ./build/bpfs
	rm -rf ./build/tests
	$(MAKE) clean -C ./bpfs

test:
	ctest --verbose --test-dir ./build -C RelWithDebInfo

config:
	cmake -B ./build -S . -G "Ninja Multi-Config" -DCMAKE_EXPORT_COMPILE_COMMANDS=1

config_no_test:
	cmake -B ./build -S . -G "Ninja Multi-Config" -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
		-DSYCLOPE_BUILD_TESTS=0

check:
	CodeChecker analyze ./build/compile_commands.json -o ./build/code-check-reports

report:
	CodeChecker parse ./build/code-check-reports -e html -o ./build/code-check-reports/html ; \
		xdg-open ./build/code-check-reports/html/index.html

.PHONY:
	reldeb debug release clean test config config_no_test check report
