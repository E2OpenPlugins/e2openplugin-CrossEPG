from enigma import getDesktop

from Screens.Screen import Screen
from Screens.MessageBox import MessageBox

from Components.config import KEY_LEFT, KEY_RIGHT, KEY_HOME, KEY_END, KEY_0, ConfigYesNo, ConfigSelection, ConfigClock, config, configfile
from Components.ConfigList import ConfigList
from Components.Button import Button
from Components.Label import Label
from Components.Harddisk import harddiskmanager
from Components.PluginComponent import plugins
from Components.ActionMap import NumberActionMap

from Tools.Directories import resolveFilename, SCOPE_PLUGINS

from Plugins.Plugin import PluginDescriptor

from crossepglib import *
from crossepg_locale import _
from crossepg_auto import crossepg_auto

from time import *

import os

class CrossEPG_Setup(Screen):
	def __init__(self, session):
		if (getDesktop(0).size().width() < 800):
			skin = "%s/skins/setup_sd.xml" % (os.path.dirname(sys.modules[__name__].__file__))
		else:
			skin = "%s/skins/setup_hd.xml" % (os.path.dirname(sys.modules[__name__].__file__))
		f = open(skin, "r")
		self.skin = f.read()
		f.close()
		Screen.__init__(self, session)

		patchtype = getEPGPatchType()
		if patchtype == 0 or patchtype == 1 or patchtype == 3:
			self.fastpatch = True
		else:
			self.fastpatch = False
		
		self.session = session

		self.config = CrossEPG_Config()
		self.config.load()

		self.lamedbs = self.config.getAllLamedbs()

		self.lamedbs_desc = []
		self.mountpoint = []
		self.mountdescription = []
		self.automatictype = []

		self.show_extension = self.config.show_extension
		self.show_plugin = self.config.show_plugin

		# make devices entries
		if self.config.isQBOXHD():
			self.mountdescription.append(_("Internal flash"))
			self.mountpoint.append("/var/crossepg/data")

		for partition in harddiskmanager.getMountedPartitions():
			if (partition.mountpoint != '/') and (partition.mountpoint != '') and self.isMountedInRW(partition.mountpoint):
				self.mountpoint.append(partition.mountpoint + "/crossepg")

				if partition.description != '':
					self.mountdescription.append(partition.description)
				else:
					self.mountdescription.append(partition.mountpoint)
				
		if not self.config.isQBOXHD():		# for other decoders we add internal flash as last entry (it's unsuggested)
			self.mountdescription.append(_("Internal flash (unsuggested)"))
			self.mountpoint.append(self.config.home_directory + "/data")
			
		# make lamedb entries
		for lamedb in self.lamedbs:
			if lamedb == "lamedb":
				self.lamedbs_desc.append(_("main lamedb"))
			else:
				self.lamedbs_desc.append(lamedb.replace("lamedb.", "").replace(".", " "))
				
		# make automatic type entries
		self.automatictype.append(_("disabled"))
		self.automatictype.append(_("once a day"))
		self.automatictype.append(_("every hour (only in standby)"))

		self.list = []
		self["config"] = ConfigList(self.list, session = self.session)
		self["config"].onSelectionChanged.append(self.setInfo)
		self["information"] = Label("")
		self["key_red"] = Button(_("Back"))
		self["key_green"] = Button()
		self["key_yellow"] = Button()
		self["key_blue"] = Button("")
		self["config_actions"] = NumberActionMap(["SetupActions", "InputAsciiActions", "KeyboardInputActions", "ColorActions"],
		{
			"red": self.quit,
			"cancel": self.quit,
			"left": self.keyLeft,
			"right": self.keyRight,
			"home": self.keyHome,
			"end": self.keyEnd,
			"1": self.keyNumberGlobal,
			"2": self.keyNumberGlobal,
			"3": self.keyNumberGlobal,
			"4": self.keyNumberGlobal,
			"5": self.keyNumberGlobal,
			"6": self.keyNumberGlobal,
			"7": self.keyNumberGlobal,
			"8": self.keyNumberGlobal,
			"9": self.keyNumberGlobal,
			"0": self.keyNumberGlobal
		}, -1) # to prevent left/right overriding the listbox

		self.makeList()

	def isMountedInRW(self, path):
		testfile = path + "/tmp-rw-test"
		os.system("touch " + testfile)
		if os.path.exists(testfile):
			os.system("rm -f " + testfile)
			return True
		return False
		
	def showWarning(self):	
		self.session.open(MessageBox, _("PLEASE READ!\nNo disk found. An hard drive or an usb pen is HARDLY SUGGESTED. If you still want use your internal flash pay attention to:\n(1) If you don't have enough free space your box may completely block and you need to flash it again\n(2) Many write operations on your internal flash may damage your flash memory"), type = MessageBox.TYPE_ERROR)
	
	def keyLeft(self):
		self["config"].handleKey(KEY_LEFT)
		self.update()
		#self.setInfo()

	def keyRight(self):
		self["config"].handleKey(KEY_RIGHT)
		self.update()
		#self.setInfo()

	def keyHome(self):
		self["config"].handleKey(KEY_HOME)
		self.update()
		#self.setInfo()

	def keyEnd(self):
		self["config"].handleKey(KEY_END)
		self.update()
		#self.setInfo()

	def keyNumberGlobal(self, number):
		self["config"].handleKey(KEY_0 + number)
		self.update()
		#self.setInfo()

	def makeList(self):
		self.list = []

		device_default = None
		i = 0
		for mountpoint in self.mountpoint:
			if mountpoint == self.config.db_root:
				device_default = self.mountdescription[i]
			i += 1

		# default device is really important... if miss a default we force it on first entry and update now the main config
		if device_default == None:
			self.config.db_root = self.mountpoint[0]
			device_default = self.mountdescription[0]
			
		lamedb_default = _("main lamedb")
		if self.config.lamedb != "lamedb":
			lamedb_default = self.config.lamedb.replace("lamedb.", "").replace(".", " ")

		scheduled_default = None
		if self.config.download_standby_enabled:
			scheduled_default = _("every hour (only in standby)")
		elif self.config.download_daily_enabled:
			scheduled_default = _("once a day")
		else:
			scheduled_default = _("disabled")

		self.list.append((_("Storage device"), ConfigSelection(self.mountdescription, device_default)))
		if len(self.lamedbs_desc) > 1:
			self.list.append((_("Preferred lamedb"), ConfigSelection(self.lamedbs_desc, lamedb_default)))

		self.list.append((_("Enable csv import"), ConfigYesNo(self.config.csv_import_enabled > 0)))
		self.list.append((_("Force epg reload on boot"), ConfigYesNo(self.config.force_load_on_boot > 0)))
		self.list.append((_("Download on tune"), ConfigYesNo(self.config.download_tune_enabled > 0)))
		self.list.append((_("Scheduled download"), ConfigSelection(self.automatictype, scheduled_default)))

		if self.config.download_daily_enabled:
			ttime = localtime()
			ltime = (ttime[0], ttime[1], ttime[2], self.config.download_daily_hours, self.config.download_daily_minutes, ttime[5], ttime[6], ttime[7], ttime[8])
			self.list.append((_("Scheduled download at"), ConfigClock(mktime(ltime))))

		if not self.fastpatch:
			self.list.append((_("Reboot after a scheduled download"), ConfigYesNo(self.config.download_daily_reboot > 0)))
			self.list.append((_("Reboot after a manual download"), ConfigYesNo(self.config.download_manual_reboot > 0)))
		self.list.append((_("Show as plugin"), ConfigYesNo(self.config.show_plugin > 0)))
		self.list.append((_("Show as extension"), ConfigYesNo(self.config.show_extension > 0)))

		self["config"].setList(self.list)
		self.setInfo()

	def update(self):
		redraw = False
		self.config.db_root = self.mountpoint[self.list[0][1].getIndex()]
		
		i = 1
		if len(self.lamedbs_desc) > 1:
			self.config.lamedb = self.lamedbs[self.list[i][1].getIndex()]
			i += 1

		self.config.csv_import_enabled = int(self.list[i][1].getValue())
		self.config.force_load_on_boot = int(self.list[i+1][1].getValue())
		self.config.download_tune_enabled = int(self.list[i+2][1].getValue())

		dailycache = self.config.download_daily_enabled
		standbycache = self.config.download_standby_enabled
		if self.list[i+3][1].getIndex() == 0:
			self.config.download_daily_enabled = 0
			self.config.download_standby_enabled = 0
		elif self.list[i+3][1].getIndex() == 1:
			self.config.download_daily_enabled = 1
			self.config.download_standby_enabled = 0
		else:
			self.config.download_daily_enabled = 0
			self.config.download_standby_enabled = 1

		if dailycache != self.config.download_daily_enabled or standbycache != self.config.download_standby_enabled:
			redraw = True

		i += 4
		if dailycache:
			self.config.download_daily_hours = self.list[i][1].getValue()[0]
			self.config.download_daily_minutes = self.list[i][1].getValue()[1]
			i += 1

		if not self.fastpatch:
			self.config.download_daily_reboot = int(self.list[i][1].getValue())
			self.config.download_manual_reboot = int(self.list[i+1][1].getValue())
			i += 2

		self.config.show_plugin = int(self.list[i][1].getValue())
		self.config.show_extension = int(self.list[i+1][1].getValue())

		if redraw:
			self.makeList()

	def setInfo(self):
		index = self["config"].getCurrentIndex()
		if len(self.lamedbs_desc) <= 1 and index > 0:
			index += 1
		if self.config.download_daily_enabled == 0 and index > 5:
			index += 1
		if self.fastpatch and index > 6:
			index += 2

		if index == 0:
			self["information"].setText(_("Drive where you save data.\nThe drive MUST be mounted in rw. If you can't see your device here probably is mounted as read only or autofs handle it only in read only mode. In case of mount it manually and try again"))
		elif index == 1:
			self["information"].setText(_("Lamedb used for epg.dat conversion.\nThis option doesn't work with crossepg patch v2"))
		elif index == 2:
			self["information"].setText(_("Import *.csv and *.bin from %s/import or %s/import\n(*.bin are binaries with a csv as stdout)") % (self.config.db_root, self.config.home_directory))
		elif index == 3:
			self["information"].setText(_("Reload epg at every boot.\nNormally it's not necessary but recover epg after an enigma2 crash"))
		elif index == 4:
			self["information"].setText(_("Only for opentv providers.\nIf you zap on channel used from a provider it download the epg in background"))
		elif index == 5:
			if self.config.download_standby_enabled:
				self["information"].setText(_("When the decoder is in standby opentv providers will be automatically downloaded every hour.\nXMLTV providers will be always downloaded only once a day"))
			elif self.config.download_daily_enabled:
				self["information"].setText(_("Download epg once a day"))
			else:
				self["information"].setText(_("Scheduled download disabled"))
		elif index == 6:
			self["information"].setText(_("Time for scheduled daily download"))
		elif index == 7:
			self["information"].setText(_("Automatically reboot the decoder after a scheduled download"))
		elif index == 8:
			self["information"].setText(_("Automatically reboot the decoder after a manual download"))
		elif index == 9:
			self["information"].setText(_("Show crossepg in plugin menu"))
		elif index == 10:
			self["information"].setText(_("Show crossepg in extensions menu"))
		
	def quit(self):
		self.config.last_full_download_timestamp = 0
		self.config.last_partial_download_timestamp = 0
		self.config.configured = 1
		self.config.save()
		try:
			config.misc.epgcache_filename.setValue(self.config.db_root + "/epg.dat")
			config.misc.epgcache_filename.callNotifiersOnSaveAndCancel = True
			config.misc.epgcache_filename.save()
			configfile.save()
		except Exception, e:
			print "custom epgcache filename not supported by current enigma2 version"
			
		if self.show_extension != self.config.show_extension or self.show_plugin != self.config.show_plugin:
			for plugin in plugins.getPlugins(PluginDescriptor.WHERE_PLUGINMENU):
				if plugin.name == "CrossEPG Downloader":
					plugins.removePlugin(plugin)
				
			for plugin in plugins.getPlugins(PluginDescriptor.WHERE_EXTENSIONSMENU):
				if plugin.name == "CrossEPG Downloader":
					plugins.removePlugin(plugin)
				
			plugins.readPluginList(resolveFilename(SCOPE_PLUGINS))
			
		crossepg_auto.forcePoll()
		
		if self.config.db_root == self.config.home_directory + "/data" and not self.config.isQBOXHD():
			self.showWarning()
			
		self.close()

