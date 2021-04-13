#!/usr/bin/python

# webif.py  by Ambrosa http://www.ambrosa.net
# this module is used for manage Web Interface
# not tested with CrossEPG

__author__ = "ambrosa http://www.ambrosa.net"
__version__ = "0.53 beta E2_LOADEPG"
__copyright__ = "Copyright (C) 2008-2009 Alessandro Ambrosini"
__license__ = "CreativeCommons by-nc-sa http://creativecommons.org/licenses/by-nc-sa/3.0/"

import time
import string
import urllib2
from urllib import quote_plus
from xml.dom import minidom


class webif_class:
    USE_WEBIF=1
    USE_WEBIF_AUTH=0
    WEBIF_AUTH_USER='root'
    WEBIF_AUTH_PASSW='qboxhd'
    WEBIF_AUTH_REALM='dm7025'
    WEBIF_IP='127.0.0.1'
    
    def __init__(self,use,auth,auth_name,auth_passw,auth_realm,ip):
        self.USE_WEBIF=use
        self.USE_WEBIF_AUTH=auth
        self.WEBIF_AUTH_USER=auth_name
        self.WEBIF_AUTH_PASSW=auth_passw
        self.WEBIF_AUTH_REALM=auth_realm
        self.WEBIF_IP=ip


    def get_use_webif(self):
        return(self.USE_WEBIF)
        
        
    # WebInterface routines
    # see http://dream.reichholf.net/wiki/Enigma2:WebInterface    
    def WI(self,command):   
        
        if self.USE_WEBIF_AUTH == 1:    
            auth_handler = urllib2.HTTPBasicAuthHandler()
            auth_handler.add_password(self.WEBIF_AUTH_REALM, 'http://' + self.WEBIF_IP, self.WEBIF_AUTH_USER, self.WEBIF_AUTH_PASSW)
            opener = urllib2.build_opener(auth_handler)
            urllib2.install_opener(opener)
            
        try:
            sock=urllib2.urlopen('http://' + self.WEBIF_IP + '/web/' + command)
            data=sock.read()
        except urllib2.URLError:
            pass
        except urllib2.HTTPError:
            pass
        except urllib2.httplib.BadStatusLine:
            pass
        else:
            sock.close()
            return(data)
            
    def standby(self):
        current_sid=self.currentchannelsid()
        if current_sid != None:
            self.WI('powerstate?newstate=0')
            time.sleep(5)
        
    def restartenigma(self):
        self.WI('powerstate?newstate=3')
        
    def switchon(self):
        current_sid=self.currentchannelsid()
        if current_sid == None:
            # DM appears switched off. Switch it on !
        
            # switch on emulating remote keypress
            # 'powerstate?newstate=116' is not (?) working
            #self.WI('remotecontrol?command=116')       
            self.WI('powerstate?newstate=0')

            time.sleep(5)
            current_sid=self.currentchannelsid()
        
        return(current_sid)


    def zap(self,channelsid):
        self.WI('zap?sRef='+channelsid)

    def reloadepgdat(self):
        self.WI('powerstate?newstate=10')

    def currentchannelsid(self):
        # DM must be SWITCHED ON
        # if DM is in standby mode, it return 'none'
        data=self.WI('subservices')
        
        if data == None:
            return(None)
            
        try:
            xmldoc = minidom.parseString(data)
        except:
            return(None)
        
        r=xmldoc.getElementsByTagName('e2servicereference')[0].firstChild.data
        if r == 'N/A':
            return(None)
            
        return(r)

    def is_recording(self):
        data=self.WI("timerlist")
        if data.find("<e2state>2</e2state>") == -1:
            return(False) # not recording
        else:
            return(True) # recording

    def message(self,txt,timeout=10,type=1):
        is_on=self.currentchannelsid()
        # WARNING: if DM is switched off, sending a message cause a lock/crash in the system
        if is_on != None:
            self.WI("message?text="+quote_plus("E2_LOADEPG - "+txt)+"&type="+str(type)+"&timeout="+str(timeout))
        

