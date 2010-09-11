import py_compile
import dircache
try:
	list = dircache.listdir("src/enigma2/python/")
	for file in list:
		if len(file) > 3 and file[-3:] == ".py":
			print file
			py_compile.compile("src/enigma2/python/%s" % (file),
								"tmp/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/%so" % (file),
								"/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/%s" % (file))
except Exception, e:
	print e
