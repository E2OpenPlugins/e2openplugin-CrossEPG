import py_compile
try:
	py_compile.compile("src/enigma2/python/__init__.py")
	py_compile.compile("src/enigma2/python/crossepg_auto.py")
	py_compile.compile("src/enigma2/python/crossepg_converter.py")
	py_compile.compile("src/enigma2/python/crossepg_downloader.py")
	py_compile.compile("src/enigma2/python/crossepg_info.py")
	py_compile.compile("src/enigma2/python/crossepg_loader.py")
	py_compile.compile("src/enigma2/python/crossepg_main.py")
	py_compile.compile("src/enigma2/python/crossepg_setup.py")
	py_compile.compile("src/enigma2/python/crossepglib.py")
	py_compile.compile("src/enigma2/python/plugin.py")
except Exception, e:
	print e
