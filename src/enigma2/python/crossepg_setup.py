from enigma import *
from crossepglib import *
from crossepg_auto import crossepg_auto
from crossepg_info import CrossEPG_Info
from crossepg_extra import CrossEPG_Extra
from crossepg_locale import _

from Screens.Screen import Screen
from Screens.MessageBox import MessageBox

from Components.config import config, getConfigListEntry, ConfigSubsection, ConfigInteger,ConfigYesNo,ConfigText,ConfigSelection,ConfigClock
from Components.ConfigList import ConfigListScreen
from Components.Label import Label
from Components.Button import Button
from Components.MenuList import MenuList
from Components.MultiContent import MultiContentEntryText
from Components.Harddisk import harddiskmanager
from Components.PluginComponent import plugins
from Components.ActionMap import ActionMap
from Tools.Directories import resolveFilename, SCOPE_PLUGINS
from Plugins.Plugin import PluginDescriptor

from time import *

import _enigma

class CrossEPG_Setup(ConfigListScreen,Screen):
	def __init__(self, session, auto_action):
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
		
		self.config = CrossEPG_Config()
		self.config.load()
		self.providers = self.config.getAllProviders()
		#self.providersdescs = self.config.getAllProvidersDescriptions()
		self.lamedbs = self.config.getAllLamedbs()
		self.lamedbs_desc = list()
		self.citems = list()
		self.mountpoint = list()
		self.mountdescription = list()
		self.session = session
		self.auto_action = auto_action
		ttime = localtime()
		ltime = (ttime[0], ttime[1], ttime[2], self.config.auto_daily_hours, self.config.auto_daily_minutes, ttime[5], ttime[6], ttime[7], ttime[8])
		default = None
		
		for partition in harddiskmanager.getMountedPartitions():
			if partition.mountpoint != "/": # and self.isMountedInRW(partition.mountpoint):
				if partition.mountpoint + "/crossepg/" == self.config.db_root:
					default = partition.description
				self.mountdescription.append(partition.description)
				self.mountpoint.append(partition.mountpoint + "/crossepg/")
				
		if len(self.mountpoint) == 0:
			self.onFirstExecBegin.append(self.noDisksFound)

		self.citems.append((_("Save data on device"), ConfigSelection(self.mountdescription, default)))
		
		for lamedb in self.lamedbs:
			if lamedb == "lamedb":
				self.lamedbs_desc.append("main lamedb")
			else:
				self.lamedbs_desc.append(lamedb.replace("lamedb.", "").replace(".", " "))
				
		if self.config.lamedb == "lamedb":
			lamedbs_sel = "main lamedb"
		else:
			lamedbs_sel = self.config.lamedb.replace("lamedb.", "").replace(".", " ")
				
		self.citems.append((_("Preferred lamedb"), ConfigSelection(self.lamedbs_desc, lamedbs_sel)))
		
		i = 0
		for provider in self.providers[0]:
			self.citems.append((_("Enable provider %s") % (self.providers[1][i]), ConfigYesNo(self.config.providers.count(provider) > 0)))
			i += 1
		
		self.citems.append((_("Enable csv import"), ConfigYesNo(self.config.enable_importer > 0)))
		self.citems.append((_("Automatic load data on boot"), ConfigYesNo(self.config.auto_boot > 0)))
		self.citems.append((_("Automatic daily download"), ConfigYesNo(self.config.auto_daily > 0)))
		self.citems.append((_("Automatic daily download at"), ConfigClock(mktime(ltime))))
		self.citems.append((_("Automatic download on tune"), ConfigYesNo(self.config.auto_tune > 0)))
		self.citems.append((_("Show OSD for automatic download"), ConfigYesNo(self.config.auto_tune_osd > 0)))
		if not self.fastpatch:
			self.citems.append((_("Reboot after a daily download"), ConfigYesNo(self.config.auto_daily_reboot > 0)))
			self.citems.append((_("Reboot after a manual download"), ConfigYesNo(self.config.manual_reboot > 0)))
		self.citems.append((_("Show as plugin"), ConfigYesNo(self.config.show_plugin > 0)))
		self.citems.append((_("Show as extension"), ConfigYesNo(self.config.show_extension > 0)))
			
		ConfigListScreen.__init__(self, self.citems)

		self["key_red"] = Button(_("Cancel"))
		self["key_green"] = Button(_("OK"))
		self["key_yellow"] = Button(_("Info"))
		self["key_blue"] = Button(_("Extra"))
		self["setupActions"] = ActionMap(["SetupActions", "ColorActions"],
		{
			"red": self.cancel,
			"green": self.saveAndQuit,
			"yellow": self.info,
			"blue": self.extra,
			"save": self.saveAndQuit,
			"cancel": self.cancel,
			"ok": self.saveAndQuit,
		}, -2)
		
	def isMountedInRW(self, mountpoint):
		try:
			mounts = open("/proc/mounts")
		except IOError:
			return False

		lines = mounts.readlines()
		mounts.close()

		for line in lines:
			if line.split(' ')[1] == mountpoint and line.split(' ')[3] == "rw":
				return True
		return False

	def noDisksFound(self):
		self.session.openWithCallback(self.close, MessageBox, _("No writable drive found. You cannot use crossepg without any writable partition."), type = MessageBox.TYPE_ERROR)

	def saveAndQuit(self):
		self.save()
		self.close()
		
	def save(self):
		reload_plugins = False
		self.config.providers = list()
		
		if len(self.mountdescription) > 0:
			self.config.db_root = self.mountpoint[self.citems[0][1].getIndex()]
			
		self.config.lamedb = self.lamedbs[self.citems[1][1].getIndex()]
		i = 2
			
		for provider in self.providers[0]:
			if self.citems[i][1].getValue() == True:
				self.config.providers.append(provider)
			i += 1
		
		self.config.enable_importer = int(self.citems[i][1].getValue())
		i += 1
		self.config.auto_boot = int(self.citems[i][1].getValue())
		i += 1
		self.config.auto_daily = int(self.citems[i][1].getValue())
		i += 1
		self.config.auto_daily_hours = self.citems[i][1].getValue()[0]
		self.config.auto_daily_minutes = self.citems[i][1].getValue()[1]
		i += 1
		self.config.auto_tune = int(self.citems[i][1].getValue())
		i += 1
		self.config.auto_tune_osd = int(self.citems[i][1].getValue())
		i += 1
		if not self.fastpatch:
			self.config.auto_daily_reboot = int(self.citems[i][1].getValue())
			i += 1
			self.config.manual_reboot = int(self.citems[i][1].getValue())
			i += 1
		
		if self.config.show_plugin != int(self.citems[i][1].getValue()):
			reload_plugins = True
		self.config.show_plugin = int(self.citems[i][1].getValue())
		i += 1
		if self.config.show_extension != int(self.citems[i][1].getValue()):
			reload_plugins = True
		self.config.show_extension = int(self.citems[i][1].getValue())
		i += 1
		
		self.config.save()
		if self.config.auto_daily:
			crossepg_auto.dailyStart()
		else:
			crossepg_auto.dailyStop()
		crossepg_auto.auto_tune = self.config.auto_tune
		crossepg_auto.auto_tune_osd = self.config.auto_tune_osd
		
		if reload_plugins:
			for plugin in plugins.getPlugins(PluginDescriptor.WHERE_PLUGINMENU):
				if plugin.name == "CrossEPG Downloader":
					plugins.removePlugin(plugin)
				
			for plugin in plugins.getPlugins(PluginDescriptor.WHERE_EXTENSIONSMENU):
				if plugin.name == "CrossEPG Downloader":
					plugins.removePlugin(plugin)
				
			plugins.readPluginList(resolveFilename(SCOPE_PLUGINS))
		
	def info(self):
		self.session.open(CrossEPG_Info)
		
	def _extraCallback(self, result):
		if result:
			self.save()
			self.session.open(CrossEPG_Extra, self.auto_action)
		
	def extra(self):
		self.session.openWithCallback(self._extraCallback, MessageBox, _("Configuration must be saved before continue. Do it now?"))
		
	def cancel(self):
		self.close()
