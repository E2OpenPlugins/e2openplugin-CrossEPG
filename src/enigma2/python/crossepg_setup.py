from enigma import *
from crossepglib import *
from crossepg_auto import crossepg_auto
from crossepg_info import CrossEPG_Info
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

from time import *

import _enigma

class CrossEPG_Setup(ConfigListScreen,Screen):
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
		if patchtype == 0 or patchtype == 1:
			self.fastpatch = True
		else:
			self.fastpatch = False
		
		self.config = CrossEPG_Config()
		self.config.load()
		self.providers = self.config.getAllProviders()
		self.lamedbs = self.config.getAllLamedbs()
		self.lamedbs_desc = list()
		self.citems = list()
		self.mountpoint = list()
		self.mountdescription = list()
		self.session = session
		ttime = localtime()
		ltime = (ttime[0], ttime[1], ttime[2], self.config.auto_daily_hours, self.config.auto_daily_minutes, ttime[5], ttime[6], ttime[7], ttime[8])
		default = None
		
		for partition in harddiskmanager.getMountedPartitions():
			if partition.mountpoint != "/":
				if partition.mountpoint + "/crossepg/" == self.config.db_root:
					default = partition.description
				self.mountdescription.append(partition.description)
				self.mountpoint.append(partition.mountpoint + "/crossepg/")
				
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
		
		for provider in self.providers:
			self.citems.append((_("Enable provider %s") % (provider), ConfigYesNo(self.config.providers.count(provider) > 0)))
		
		self.citems.append((_("Enable csv import"), ConfigYesNo(self.config.enable_importer > 0)))
		self.citems.append((_("Automatic load data on boot"), ConfigYesNo(self.config.auto_boot > 0)))
		self.citems.append((_("Automatic daily download"), ConfigYesNo(self.config.auto_daily > 0)))
		self.citems.append((_("Automatic daily download at"), ConfigClock(mktime(ltime))))
		self.citems.append((_("Automatic download on tune"), ConfigYesNo(self.config.auto_tune > 0)))
		self.citems.append((_("Show OSD for automatic download"), ConfigYesNo(self.config.auto_tune_osd > 0)))
		if not self.fastpatch:
			self.citems.append((_("Reboot after a daily download"), ConfigYesNo(self.config.auto_daily_reboot > 0)))
			self.citems.append((_("Reboot after a manual download"), ConfigYesNo(self.config.manual_reboot > 0)))
		self.citems.append((_("Show downloader in plugin list"), ConfigYesNo(self.config.show_downloader > 0)))
			
		ConfigListScreen.__init__(self, self.citems)
		self["key_red"] = Button(_("Cancel"))
		self["key_green"] = Button(_("OK"))
		self["key_yellow"] = Button(_("Info"))
		self["key_blue"] = Button(_(" "))
		self["setupActions"] = ActionMap(["SetupActions", "ColorActions"],
		{
			"red": self.cancel,
			"green": self.save,
			"yellow": self.info,
			"save": self.save,
			"cancel": self.cancel,
			"ok": self.save,
		}, -2)
		
	def save(self):
		reload_plugins = False
		self.config.providers = list()
		
		self.config.db_root = self.mountpoint[self.citems[0][1].getIndex()]
		self.config.lamedb = self.lamedbs[self.citems[1][1].getIndex()]
		i = 2
			
		for provider in self.providers:
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
		self.config.show_downloader = int(self.citems[i][1].getValue())
		i += 1
		
		self.config.save()
		if self.config.auto_daily:
			crossepg_auto.dailyStart()
		else:
			crossepg_auto.dailyStop()
		crossepg_auto.auto_tune = self.config.auto_tune
		crossepg_auto.auto_tune_osd = self.config.auto_tune_osd
		
		plugins.readPluginList(resolveFilename(SCOPE_PLUGINS))
		
		self.close()
		
	def info(self):
		self.session.open(CrossEPG_Info)
		
	def cancel(self):
		self.close()