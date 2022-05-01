#! /usr/bin/env python


import os
import sys
import uuid
from math import pow

def enum(*sequential, **named):
   enums = dict(zip(sequential, range(len(sequential))), **named)
   reverse = dict((value, key) for key, value in enums.iteritems())
   enums['reverse_mapping'] = reverse
   return type('Enum', (), enums)

class UiElement:
   name = ''
   ident = ''
   description = None
   parent = None
   children = None


class Group(UiElement):
   collapse = True
   

   def __init__(self, name, collapse=True, description=None, ident=None):
      self.name = name
      self.collapse = collapse
      self.description = description
      
      if ident==None:
         ident = name.replace(' ', '')
         ident = ident.replace('_', '')
         ident = ident.lower()
         temptrans = "".maketrans('','')
         # self.ident = ident.translate(temptrans,'aeiouy')
         
      else:
         self.ident = ident

      self.unique_name = None

   def __str__(self):
      return 'Group %s' % self.name


class Tab(Group):
   def __init__(self, name, collapse=True, description=None,  ident=None):
      self.name = name
      self.collapse = collapse
      self.description = description
      
      if ident==None:
         ident = name.replace(' ', '')
         ident = ident.replace('_', '')
         ident = ident.lower()
         temptrans = "".maketrans('','')
         # self.ident = ident.translate(temptrans,'aeiouy')
      else:
         self.ident = ident

      self.unique_name = None

   def __str__(self):
      return 'Tab %s' % self.name

class Parameter(UiElement):
   ptype = None
   label = None
   mn = None
   mx = None
   smn = None
   smx = None
   connectible = True
   enum_names = None
   default = None
   fig = None
   figc = None
   short_description = ''
   presets = None
   mayane = False
   ui = ''
   houdini_disable_when = None
   houdini_join_next = None
   houdini_default = None
   maya_hide = None
   filePathBrowse = None

   def __init__(self, name, ptype, default, label=None, description=None, mn=None, mx=None, smn=None, smx=None, connectible=True, enum_names=None, fig=None, figc=None, presets={}, mayane=False, ui='', houdini_disable_when=None, houdini_join_next=None, houdini_default=None, maya_hide=None, filePathBrowse=None):
      self.name = name
      self.ptype = ptype
      self.default = default
      self.description = description
      self.presets = presets
      self.mayane = mayane
      self.ui = ui
      self.houdini_disable_when = houdini_disable_when
      self.houdini_join_next = houdini_join_next
      self.houdini_default = houdini_default
      self.maya_hide = maya_hide
      self.filePathBrowse = filePathBrowse

      if description is not None:
         if len(description) > 150:
            # build the short description, < 200 chars
            sentences = description.split('.')
            sdlen = 0
            for s in sentences:
               self.short_description += s + "."
               sdlen += len(s)
               if sdlen > 150:
                  break
         else:
            self.short_description = description

      if not label:
         label = name   
      self.label = label   
      self.mn = mn
      self.mx = mx
      self.smn = smn
      self.smx = smx
      self.fig = fig
      self.figc = figc

      if ptype not in ('bool', 'int', 'float', 'rgb', 'vector', 'string', 'enum', 'matrix'):
         raise ValueError('parameter %s has unrecognized ptype %r' % (name, ptype))

      if ptype == 'enum':
         self.enum_names = enum_names

      if ptype == 'bool' or ptype == 'string' or ptype == 'int' or ptype == 'enum':
         self.connectible = False
      else:
         self.connectible = connectible

      # do a quick sanity check on the ptype and default
      if ptype == 'bool' and type(default) is not bool:
         raise ValueError('parameter %s was typed as %s but default had type %r' % (name, ptype, type(default)))
      elif ptype in ('rgb', 'vector') and type(default) is not tuple:
         raise ValueError('parameter %s was typed as %s but default had type %r' % (name, ptype, type(default)))
      elif ptype in ('string', 'enum') and type(default) is not str:
         raise ValueError('parameter %s was typed as %s but default had type %r' % (name, ptype, type(default)))

      self.default = default

   def __str__(self):
      return 'Parameter %s %s' % (self.name, self.ptype)

class AOV(Parameter):
   def __init__(self, name, ptype, label=None):
      self.name = name
      self.ptype = ptype
      self.label = label
      self.connectible = False

   def __str__(self):
      return 'AOV %s %s' % (self.name, self.ptype)

class ShaderDef:
   name = None
   intro = None
   description = None
   output = None
   aov_shader = False
   c4d_classification = None
   c4d_menu = None
   c4d_command_id = None
   maya_name = None
   maya_classification = None
   maya_translator = None
   maya_attr_prefix = None
   maya_id = None
   maya_swatch = False
   maya_bump = False
   maya_matte = False
   soft_name = None
   soft_classification = None
   help_url = None
   houdini_icon = None
   houdini_category = None
   soft_version = 1
   hierarchy_depth = 0
   root = None
   current_parent = None
   parameters = []
   aovs = []
   groups = []
   tabs = []

   def __init__(self):
      self.root = Group('__ROOT__', False)
      self.current_parent = self.root

   def setMayaClassification(self, mc):
      self.maya_classification = mc

   def setDescription(self, d):
      self.description = d

   def setOutput(self, o):
      self.output = o

   def beginGroup(self, name, collapse=True, description='', ident=None):
      g = None
      if self.current_parent is self.root:
         g = Tab(name, collapse, description,ident)
      else:
         g = Group(name, collapse, description,ident)

      if not self.current_parent.children:
         self.current_parent.children = [g]
      else:
         self.current_parent.children.append(g)

      g.parent = self.current_parent
      self.current_parent = g

   def endGroup(self):
      self.current_parent = self.current_parent.parent

   def parameter(self, name, ptype, default, label=None, description=None, mn=None, mx=None, smn=None, smx=None, connectible=True, enum_names=None, fig=None, figc = None, presets=None, mayane=False, ui='', houdini_disable_when=None, houdini_join_next=None, houdini_default=None, maya_hide=None, filePathBrowse=None):
      p = Parameter(name, ptype, default, label, description, mn, mx, smn, smx, connectible, enum_names, fig, figc, presets, mayane, ui, houdini_disable_when, houdini_join_next, houdini_default, maya_hide, filePathBrowse)
      if not self.current_parent.children:
         self.current_parent.children = [p]
      else:
         self.current_parent.children.append(p)

      p.parent = self.current_parent
      self.parameters.append(p)

   def aov(self, name, ptype, label=None, description=None):
      p = AOV(name, ptype, label)
      if not self.current_parent.children:
         self.current_parent.children = [p]
      else:
         self.current_parent.children.append(p)

      p.parent = self.current_parent
      self.aovs.append(p)

   def __str__(self):
      return 'ShaderDef %s' % self.name

   def shader(self, d):
      if 'name' in d.keys():
         self.name = d['name']
      if 'intro' in d.keys():
         self.intro = d['intro']
      if 'description' in d.keys():
         self.description = d['description']
      if 'output' in d.keys():
         self.output = d['output']
      if 'aov_shader' in d.keys():
         self.aov_shader = d.get('aov_shader')
      if 'c4d_classification' in d.keys():
         self.c4d_classification = d.get('c4d_classification')
      if 'c4d_menu' in d.keys():
         self.c4d_menu = d.get('c4d_menu')
      if 'c4d_command_id' in d.keys():
         self.c4d_command_id = d.get('c4d_command_id')
      if 'maya_name' in d.keys():
         self.maya_name = d['maya_name']
      if 'maya_classification' in d.keys():
         self.maya_classification = d['maya_classification']
      if 'maya_translator' in d.keys():
         self.maya_translator = d['maya_translator']
      if 'maya_attr_prefix' in d.keys():
         self.maya_attr_prefix = d['maya_attr_prefix']
      if 'maya_id' in d.keys():
         self.maya_id = d['maya_id']
      if 'maya_swatch' in d.keys():
         self.maya_swatch = d['maya_swatch']
      if 'maya_bump' in d.keys():
         self.maya_bump = d['maya_bump']
      if 'maya_matte' in d.keys():
         self.maya_matte = d['maya_matte']
      if 'soft_name' in d.keys():
         self.soft_name = d['soft_name']
      if 'soft_classification' in d.keys():
         self.soft_classification = d['soft_classification']
      if 'soft_version' in d.keys():
         self.soft_version = d['soft_version']
      if 'help_url' in d.keys():
         self.help_url = d['help_url']
      else:
         self.help_url = 'https://bitbucket.org/anderslanglands/alshaders/wiki/Home'
      if 'houdini_icon' in d.keys():
         self.houdini_icon = d['houdini_icon']
      if 'houdini_category' in d.keys():
         self.houdini_category = d['houdini_category']


   #all groups need unique ident in houdini
   def uniqueGroupIdents(self):
      grpidents = []    
      for grp in self.groups:
         oldident=grp.ident
         if grp.ident in grpidents:
            grp.ident = '%s%d' % (grp.ident, grpidents.count(oldident))
         grpidents.append(oldident)    

class group:
   name = ''
   collapse = True
   description = ''
   ui = None
   ident = None

   def __init__(self, ui, name, collapse=True, description=None, ident=None):
      self.ui = ui
      self.name = name     
      self.collapse = collapse
      self.description = description
      self.ident = ident

   def __enter__(self):
      self.ui.beginGroup(self.name, self.collapse, self.description, self.ident)

   def __exit__(self, type, value, traceback):
      self.ui.endGroup()


def DebugPrintElement(el, d):
   indent = ''
   for i in range(d):
      indent += '\t'
   print('%s %s' % (indent, el))
   if isinstance(el, Group) and el.children:
      print('%s has %d children' % (el, len(el.children)))
      for e in el.children:
         DebugPrintElement(e, d+1)
      d -= 1

def DebugPrintShaderDef(sd):
   print('%s' % sd)
   DebugPrintElement(sd.root, 0)

def writei(f, s, d=0):
   indent = ''
   for i in range(d):
      indent += '\t'
   f.write('%s%s\n' %(indent, s))

def WalkAETemplate(el, f, d):
   if isinstance(el, Group):
      writei(f, 'self.beginLayout("%s", collapse=%r)' % (el.name, el.collapse), d)

      if el.children:
         for e in el.children:
            WalkAETemplate(e, f, d)

      writei(f, 'self.endLayout() # END %s' % el.name, d)
   elif isinstance(el, AOV):
      writei(f, 'self.addControl("%s", label="%s", annotation="%s")' % (el.name, el.label, el.short_description), d)
   elif isinstance(el, Parameter):
      if el.ptype == 'float':
         #writei(f, 'self.addCustomFlt("%s")' % el.name, d)
         writei(f, 'self.addControl("%s", label="%s", annotation="%s")' % (el.name, el.label, el.short_description), d)
      elif el.ptype == 'rgb':
         writei(f, 'self.addCustomRgb("%s")' % el.name, d)
      else:
         writei(f, 'self.addControl("%s", label="%s", annotation="%s")' % (el.name, el.label, el.short_description), d)

def WalkAETemplateParams(el, f, d):
   if isinstance(el, Group):
      if el.children:
         for e in el.children:
            WalkAETemplateParams(e, f, d)

   elif isinstance(el, Parameter):
      writei(f, 'self.params["%s"] = Param("%s", "%s", "%s", "%s", presets=%s)' % (el.name, el.name, el.label, el.short_description, el.ptype, el.presets), d)

def WriteAETemplate(sd, fn):
   f = open(fn, 'w')
   writei(f, 'import mtoa.ui.ae.templates as templates', 0)
   writei(f, 'import maya.cmds as cmds', 0)
   writei(f, 'import mtoa.ui.ae.utils as aeUtils', 0)
   writei(f, 'from mtoa.ui.ae.shaderTemplate import ShaderAETemplate', 0)

   writei(f, 'class AE%sTemplate(templates.AttributeTemplate):' % sd.name, 0)
   # writei(f, 'controls = {}', 1)
   # writei(f, 'params = {}', 1)
   writei(f, 'def filenameEditBokehInput(self, mData) :', 1)
   writei(f, "attr = self.nodeAttr('bokehImagePath')", 2)
   writei(f, "cmds.setAttr(attr,mData,type='string')", 2)
   writei(f, '')

   writei(f, 'def LoadFilenameButtonPushBokehInput(self, *args):', 1)
   writei(f, "basicFilter = 'All Files (*.*)'", 2)
   writei(f, "ret = cmds.fileDialog2(fileFilter=basicFilter, dialogStyle=2, cap='Select file location',fm=0)", 2)
   writei(f, "if ret is not None and len(ret):", 2)
   writei(f, "self.filenameEditBokehInput(ret[0])", 3)
   writei(f, "cmds.textFieldButtonGrp('filenameBokehGrpInput', edit=True, text=ret[0])", 3)
   writei(f, '')
    
        
   writei(f, 'def filenameNewBokehInput(self, nodeName):', 1)
   writei(f, "path = cmds.textFieldButtonGrp('filenameBokehGrpInput', label='Bokeh Image', changeCommand=self.filenameEditBokehInput, width=300)", 2)
   writei(f, "cmds.textFieldButtonGrp(path, edit=True, text=cmds.getAttr(nodeName))", 2)
   writei(f, "cmds.textFieldButtonGrp(path, edit=True, buttonLabel='...', buttonCommand=self.LoadFilenameButtonPushBokehInput)", 2)
   writei(f, '')

    
   writei(f, 'def filenameReplaceBokehInput(self, nodeName):', 1)
   writei(f, "cmds.textFieldButtonGrp('filenameBokehGrpInput', edit=True, text=cmds.getAttr(nodeName) )", 2)
   writei(f, '')
        
   
   writei(f, 'def setup(self):', 1)

   # writei(f, 'self.params.clear()', 2)
   # for e in sd.root.children:
   #    WalkAETemplateParams(e, f, 2)
   
   writei(f, '')

   if sd.maya_swatch:
      writei(f, 'self.addSwatch()', 2)

   # writei(f, 'self.beginScrollLayout()', 2) # begin main scrollLayout
   writei(f, '')

   for e in sd.root.children:
      WalkAETemplate(e, f, 2)

   if sd.maya_bump:
      writei(f, 'self.addBumpLayout()', 2)      

   writei(f, '')
   # writei(f, 'pm.mel.AEdependNodeTemplate(self.nodeName)', 2)

   # writei(f, 'self.addExtraControls()', 2)

   # writei(f, '')
   # writei(f, 'self.endScrollLayout()', 2) #end main scrollLayout

   writei(f, '')
   writei(f, 'templates.registerTranslatorUI(AE{}Template, "{}", "{}")'.format(sd.name, sd.maya_classification, sd.name), 0)
   

   f.close()

def toMayaType(ptype):
   if ptype == 'rgb' or ptype == 'vector':
      return 'float3'
   else:
      return ptype

def WalkAEXMLAttributes(el, f, d):
   if isinstance(el, Group):
      if el.children:
         for e in el.children:
            WalkAEXMLAttributes(e, f, d)
   elif isinstance(el, Parameter):
      writei(f, '<attribute name="%s" type="maya.%s">' % (el.name, toMayaType(el.ptype)), 2)
      writei(f, '<label>%s</label>' % el.label, 3)
      writei(f, '<description>%s</description>' % el.short_description, 3)
      writei(f, '</attribute>', 2)

def WalkNEXMLAttributes(el, f, d):
   if isinstance(el, Group):
      if el.children:
         for e in el.children:
            WalkNEXMLAttributes(e, f, d)
   elif isinstance(el, Parameter) and el.mayane:
      writei(f, '<attribute name="%s" type="maya.%s">' % (el.name, toMayaType(el.ptype)), 2)
      writei(f, '<label>%s</label>' % el.label, 3)
      writei(f, '</attribute>', 2)

def WalkNEXMLView(el, f, d):
   if isinstance(el, Group):
      if el.children:
         for e in el.children:
            WalkNEXMLView(e, f, d)
   elif isinstance(el, Parameter) and el.mayane:
      writei(f, '<property name="%s" />' % el.name, 2)

def WalkAEXMLGroups(el, f, d):
   if isinstance(el, Group):
      writei(f, '<group name="%s">' % ''.join(el.name.split()), d)
      writei(f, '<label>%s</label>' % el.name, d+1)
      if el.children:
         for e in el.children:
            WalkAEXMLGroups(e, f, d+1)
      writei(f, '</group>', d)
   elif isinstance(el, Parameter):
      writei(f, '<property name="%s"/>' % el.name, d)

def WriteNEOutputAttr(sd, f, d):
   if sd.output == 'rgb':
      writei(f, '<attribute name="outColor" type="maya.float3">', d)
      writei(f, '<label>Out Color</label>', d)
      writei(f, '</attribute>')
   elif sd.output == 'rgba':
      writei(f, '<attribute name="outColor" type="maya.float3">', d)
      writei(f, '<label>Out Color</label>', d)
      writei(f, '</attribute>')
      writei(f, '<attribute name="outAlpha" type="maya.float">', d)
      writei(f, '<label>Out Alpha</label>', d)
      writei(f, '</attribute>')
   elif sd.output == 'vector':
      writei(f, '<attribute name="outValue" type="maya.float3">', d)
      writei(f, '<label>Out Value</label>', d)
      writei(f, '</attribute>')
   elif sd.output == 'float':
      writei(f, '<attribute name="outValue" type="maya.float">', d)
      writei(f, '<label>Out Value</label>', d)
      writei(f, '</attribute>')

def WriteNEOutputProp(sd, f, d):
   if sd.output == 'rgb':
      writei(f, '<property name="outColor" />', d)
   elif sd.output == 'rgba':
      writei(f, '<property name="outColor" />', d)
      writei(f, '<property name="outAlpha" />', d)
   elif sd.output == 'vector':
      writei(f, '<property name="outValue" />', d)
   elif sd.output == 'float':
      writei(f, '<property name="outValue" />', d)

def WriteAEXML(sd, fn):
   f = open(fn, 'w')
   writei(f, '<?xml version="1.0" encoding="UTF-8"?>', 0)
   writei(f, '<templates>', 1)

   writei(f, '<template name="AE%s">' % sd.maya_name, 1)
   writei(f, '<label>%s</label>' % sd.maya_name, 2)
   writei(f, '<description>%s</description>' % sd.description, 2)

   for e in sd.root.children:
      WalkAEXMLAttributes(e, f, 2)
   writei(f, '</template>', 1)

   writei(f, '<view name="Lookdev" template="AE%s">' % sd.maya_name, 2)
   for e in sd.root.children:
      WalkAEXMLGroups(e, f, 3)
   writei(f, '</view>', 2)

   writei(f, '</templates>', 0)

def WriteNEXML(sd, fn):
   f = open(fn, 'w')
   writei(f, '<?xml version="1.0" encoding="UTF-8"?>', 0)
   writei(f, '<templates>', 1)

   writei(f, '<template name="NE%s">' % sd.maya_name, 1)
   WriteNEOutputAttr(sd, f, 2)
   for e in sd.root.children:
      WalkNEXMLAttributes(e, f, 2)
   writei(f, '</template>', 1)

   writei(f, '<view name="NEDefault" template="NE%s">' % sd.maya_name, 2)
   WriteNEOutputProp(sd, f, 2)
   for e in sd.root.children:
      WalkNEXMLView(e, f, 2)
   writei(f, '</view>', 2)

   writei(f, '</templates>', 0)

#########
# C4DtoA
#########

# Parameter ids are generated from the name.
def GenerateC4DtoAId(name, parameter_name):
   unique_name = "%s.%s" % (name, parameter_name)

   pid = 5381
   for c in unique_name:
      pid = ((pid << 5) + pid) + ord(c) # hash*33 + c
      #pid = pid*33 + ord(c)
      #print "%d %c" % (pid, c)

   # convert to unsigned int
   pid = pid & 0xffffffff
   if pid > 2147483647: pid = 2*2147483647 - pid + 2

   return pid

# Writes out group ids to the header file.
def WalkC4DtoAHeaderGroups(el, f, name):
   if isinstance(el, Group):
      group_name = el.unique_name if el.unique_name else el.name
      writei(f, 'C4DAI_%s_%s_GRP,' % (name.upper(), group_name.upper().replace(' ', '_')), 1)

      if el.children:
         for e in el.children:
            WalkC4DtoAHeaderGroups(e, f, name)

# Writes out parameter ids to the header file.
def WalkC4DtoAHeaderParameters(el, f, name):
   if isinstance(el, Group):
      if el.children:
         for e in el.children:
            WalkC4DtoAHeaderParameters(e, f, name)

   # skip aovs
   #if isinstance(el, AOV):
   #  pass

   if isinstance(el, Parameter):
      pid = GenerateC4DtoAId(name, el.name)
      writei(f, 'C4DAIP_%s_%s = %d,' % (name.upper(), el.name.upper().replace(' ', '_'), pid), 1)

# Writes .h header file.
def WriteC4DtoAHeaderFile(sd, name, build_dir):
   path = os.path.join(build_dir, "C4DtoA", "res", "description", "aitag_%s.h" % name)
   if os.path.exists(path):
      os.remove(path)
   if not os.path.exists(os.path.dirname(path)):
      os.makedirs(os.path.dirname(path))
   f = open(path, 'w')

   writei(f, '#ifndef _aitag_%s_h_' % name, 0)
   writei(f, '#define _aitag_%s_h_' % name, 0) 
   writei(f, '', 0) 
   writei(f, 'enum', 0) 
   writei(f, '{', 0) 
   writei(f, 'C4DAI_%s_MAIN_GRP = 2001,' % (name.upper()), 1)

   # groups
   for e in sd.root.children:
      WalkC4DtoAHeaderGroups(e, f, name)
   # matte
   if sd.maya_matte:
      writei(f, 'C4DAI_%s_MATTE_GRP,' % name.upper(), 1)   

   writei(f, '', 0)

   # parameters
   for e in sd.root.children:
      WalkC4DtoAHeaderParameters(e, f, name)
   # matte
   if sd.maya_matte:
      writei(f, 'C4DAIP_%s_AIENABLEMATTE = %d,' % (name.upper(), GenerateC4DtoAId(name, 'aiEnableMatte')), 1)
      writei(f, 'C4DAIP_%s_AIMATTECOLOR = %d,' % (name.upper(), GenerateC4DtoAId(name, 'aiMatteColor')), 1)
      writei(f, 'C4DAIP_%s_AIMATTECOLORA = %d,' % (name.upper(), GenerateC4DtoAId(name, 'aiMatteColorA')), 1)

   writei(f, '};', 0) 
   writei(f, '', 0) 
   writei(f, '#endif', 0) 
   writei(f, '', 0) 

# Writes out layout to the resource file.
def WalkC4DtoARes(el, f, name, d):
   if isinstance(el, Group):
      group_name = el.unique_name if el.unique_name else el.name      
      writei(f, 'GROUP C4DAI_%s_%s_GRP' % (name.upper(), group_name.upper().replace(' ', '_')), d)
      writei(f, '{', d)

      if not el.collapse:
         writei(f, 'DEFAULT 1;', d+1)
         writei(f, '', 0)

      if el.children:
         for e in el.children:
            WalkC4DtoARes(e, f, name, d+1)

      writei(f, '}', d)
      writei(f, '', 0)

   # skip aovs
   #elif isinstance(el, AOV):
   #  pass

   elif isinstance(el, Parameter):
      writei(f, 'AIPARAM C4DAIP_%s_%s {}' % (name.upper(), el.name.upper().replace(' ', '_')), d)

# Writes .res resource file.
def WriteC4DtoAResFile(sd, name, build_dir):
   path = os.path.join(build_dir, "C4DtoA", "res", "description", "aitag_%s.res" % name)
   if os.path.exists(path):
      os.remove(path)
   if not os.path.exists(os.path.dirname(path)):
      os.makedirs(os.path.dirname(path))
   f = open(path, 'w')

   writei(f, 'CONTAINER AITAG_%s' % name.upper(), 0)
   writei(f, '{', 0)
   writei(f, 'NAME aitag_%s;' % name, 1)
   writei(f, '', 0)
   writei(f, 'INCLUDE GVbase;', 1)
   writei(f, '', 0)
   writei(f, 'GROUP C4DAI_%s_MAIN_GRP' % name.upper(), 1)
   writei(f, '{', 1)
   writei(f, 'DEFAULT 1;', 2)
   writei(f, '', 0)

   # matte
   if sd.maya_matte:
      writei(f, 'GROUP C4DAI_%s_MATTE_GRP' % name.upper(), 2)
      writei(f, '{', 2)
      writei(f, 'AIPARAM C4DAIP_%s_AIENABLEMATTE {}' % name.upper(), 3)
      writei(f, 'AIPARAM C4DAIP_%s_AIMATTECOLOR {}' % name.upper(), 3)
      writei(f, 'AIPARAM C4DAIP_%s_AIMATTECOLORA {}' % name.upper(), 3)
      writei(f, '}', 2)
      writei(f, '', 0)

   # groups and parameters
   for e in sd.root.children:
      WalkC4DtoARes(e, f, name, 2)

   writei(f, '}', 1)
   writei(f, '}', 0)
   writei(f, '') 

# Writes out group labels to the string file.
def WalkC4DtoAStringGroups(el, f, name):
   if isinstance(el, Group):
      group_name = el.unique_name if el.unique_name else el.name
      writei(f, 'C4DAI_%s_%s_GRP   "%s";' % (name.upper(), group_name.upper().replace(' ', '_'), el.name), 1)

      if el.children:
         for e in el.children:
            WalkC4DtoAStringGroups(e, f, name)

# Writes out parameter labels to the string file.
def WalkC4DtoAStringParameters(el, f, name):
   if isinstance(el, Group):
      if el.children:
         for e in el.children:
            WalkC4DtoAStringParameters(e, f, name)

   if isinstance(el, Parameter) and el.label:
      writei(f, 'C4DAIP_%s_%s   "%s";' % (name.upper(), el.name.upper(), el.label), 1)

# Writes .str string file.
def WriteC4DtoAStringFile(sd, name, build_dir):
   path = os.path.join(build_dir, "C4DtoA", "res", "strings_us", "description", "aitag_%s.str" % name)
   if os.path.exists(path):
      os.remove(path)
   if not os.path.exists(os.path.dirname(path)):
      os.makedirs(os.path.dirname(path))
   f = open(path, 'w')

   writei(f, 'STRINGTABLE aitag_%s' % name, 0)
   writei(f, '{', 0)
   writei(f, 'aitag_%s   "Arnold %s node";' % (name, name), 1) 
   writei(f, '', 0)
   writei(f, 'C4DAI_%s_MAIN_GRP   "Main";' % (name.upper()), 1) 

   # groups
   for e in sd.root.children:
      WalkC4DtoAStringGroups(e, f, name)
   # matte
   if sd.maya_matte:
      writei(f, 'C4DAI_%s_MATTE_GRP   "Matte";' % name.upper(), 1)

   writei(f, '', 0)

   # parameters
   for e in sd.root.children:
      WalkC4DtoAStringParameters(e, f, name)
   # matte
   if sd.maya_matte:
      writei(f, 'C4DAIP_%s_AIENABLEMATTE   "Enable matte";' % name.upper(), 1)
      writei(f, 'C4DAIP_%s_AIMATTECOLOR   "Matte color";' % name.upper(), 1)
      writei(f, 'C4DAIP_%s_AIMATTECOLORA   "Matte opacity";' % name.upper(), 1)

   writei(f, '}', 0)
   writei(f, '', 0)

# Writes resource files for C4DtoA.
# To describe the UI we need three resource files:
#  - .h header file which contains parameter ids
#  - .res resource file which contains widget layout
#  - .str string file which contains labels
def WriteC4DtoAResourceFiles(sd, name, build_dir):
   # NOTE group list is build in WriteMTD
   # create unique group names
   names = set()
   groups = sd.tabs[:]
   groups.extend(sd.groups)
   for group in groups:
      if group.name not in names:
         names.add(group.name)
         continue

      i = 1
      group.unique_name = group.name
      while group.unique_name in names:
         group.unique_name = group.name + " %d" % i
         i += 1
      names.add(group.unique_name)

   # header file
   WriteC4DtoAHeaderFile(sd, name, build_dir)
   # resource file
   WriteC4DtoAResFile(sd, name, build_dir)
   # string file
   WriteC4DtoAStringFile(sd, name, build_dir)


#make list of groups and tabs
def buildGroupList(sd, el):   
   if isinstance(el, Group):
      if not isinstance(el, Tab):      
         sd.groups.append(el)

   if isinstance(el, Tab):    
      sd.tabs.append(el)

   if el.children:
      for e in el.children:
         buildGroupList(sd, e)
   

#find the total number of child groups and parameters of a Tab
def findTotalChildGroups(tab):
   if tab.children:
      totalchildren=len(tab.children)  
   
      for t in tab.children:
         totalchildren+=findTotalChildGroups(t)

      return totalchildren
   else:
      return 0 


#print ordered list of all parameters and groups   
def writeParameterOrder(f, grp, first=1, orderc=1):
   firstfolder=first
   ordercount=orderc
   for gr in grp.children:
      if isinstance(gr, Parameter):
         f.write('%s ' % gr.name)

      if isinstance(gr, Group) and not isinstance(gr, Tab):
         f.write('%s ' % gr.ident)
         writeParameterOrder(f,gr,firstfolder,ordercount)
      
      if isinstance(gr, Tab):
         if firstfolder == 1:
            f.write('folder1 ')
            firstfolder=0
         ordercount+=1
         f.write('"\n\thoudini.order%d STRING "' % ordercount)
      
         writeParameterOrder(f,gr, firstfolder,ordercount)  

class HoudiniEntry:
   name = ''
   etype = ''
   def __init__(self, nm, et):
      self.name = nm
      self.etype = et

class HoudiniFolder:
   name = ''
   group_list = []

def WalkHoudiniHeader(grp, group_list):
   for el in grp.children:
      if isinstance(el, Group):
         group_list.append(HoudiniEntry(el.name, 'group'))
         WalkHoudiniHeader(el, group_list)
      elif isinstance(el, Parameter):
         group_list.append(HoudiniEntry(el.name, 'param'))

def WriteHoudiniHeader(sd, f):
   root_order_list = []
   folder_ROOT_list = []
   for el in sd.root.children:
      if isinstance(el, Group):
         new_folder = HoudiniFolder()
         new_folder.name = el.name
         group_list = []
         WalkHoudiniHeader(el, group_list)
         new_folder.group_list = group_list
         folder_ROOT_list.append(new_folder)
      elif isinstance(el, Parameter):
         root_order_list.append(HoudiniEntry(el.name, 'param'))
      

   groupcounter = []
   counter = 0
   listening = False
   for el in folder_ROOT_list:
      # print el.name
      listening = False
      for el2 in el.group_list:
         
         if el2.etype == 'param':
            if listening:
               counter += 1
         if el2.etype == 'group':
            
            if listening:
               groupcounter.append(counter)
               counter = 0

            listening=True
         # print ' ----- ', el2.name, ', ', el2.etype
      
      if counter > 0:
         groupcounter.append(counter)
      counter = 0
            
   # print groupcounter
   


   # tell houdini how many groups we have and how big they are. in advance. because houdini can be dumb too sometimes
   groups_list = []
   if len(folder_ROOT_list):
      idx = 0
      for folder in folder_ROOT_list:
         folder_ROOT_list_STR = 'houdini.parm.group.g%d STRING "' % idx
         folder_ROOT_list_STR += '%s;%d' % (folder.name, len(folder.group_list))
         folder_ROOT_list_STR += '"'
         writei(f, folder_ROOT_list_STR, 1)
         groups_list.append('g%d'%idx)
         idx += 1
         

   # now tell houdini about all the headings we have...


   heading_list = []
   heading_idx = 0
   for folder in folder_ROOT_list:
      for el in folder.group_list:
         if el.etype == 'group':
            header_str = 'houdini.parm.group.c%d STRING "%s;%d"' % (heading_idx, el.name, groupcounter[heading_idx])
            heading_idx += 1
            #writei(f, header_str, 1)


   # now tell houdini what's actually in the damn groups
   order_idx = 2
   heading_idx = 0
   folder_idx = 0
   for folder in folder_ROOT_list:
      if folder_idx == 0:
         order_str = 'houdini.order STRING "%s' % groups_list[folder_idx]
      else:
         order_str = '"%s' % groups_list[folder_idx]
      folder_idx += 1
      for el in folder.group_list:
         # if el.etype == 'group':
         #    order_str = '%s c%d' % (order_str, heading_idx)
         #    heading_idx += 1
         # else:
            order_str = '%s %s' % (order_str, el.name)
      order_str += ' "'
      writei(f, order_str, 1)
      order_idx += 1



def WriteMDTHeader(sd, f): 
   writei(f, '[node %s]' % sd.name, 0)
   writei(f, 'desc STRING "%s"' % sd.description, 1)
   if sd.aov_shader: WriteMTDParam(f, 'aov_shader', 'bool', sd.aov_shader, 1)
   if sd.c4d_classification: writei(f, 'c4d.classification STRING "%s"' % sd.c4d_classification, 1)
   if sd.c4d_menu: writei(f, 'c4d.menu STRING "%s"' % sd.c4d_menu, 1)
   if sd.c4d_command_id: writei(f, 'c4d.command_id INT %s' % sd.c4d_command_id, 1)
   writei(f, 'maya.name STRING "%s"' % sd.maya_name, 1)
   writei(f, 'maya.classification STRING "%s"' % sd.maya_classification, 1)
   if sd.maya_translator is not None:
      writei(f, 'maya.translator STRING "%s"' % sd.maya_translator, 1)
   # if sd.maya_attr_prefix is not None:
   writei(f, 'maya.attr_prefix STRING ""', 1)
   writei(f, 'maya.id INT %s' % sd.maya_id, 1)
   #writei(f, 'houdini.label STRING "%s"' % sd.name, 1)
   if sd.houdini_icon is not None:
      writei(f, 'houdini.icon STRING "%s"' % sd.houdini_icon, 1)
   hcat = ("alShaders" if sd.houdini_category is None else sd.houdini_category)
   writei(f, 'houdini.category STRING "%s"' % hcat ,1)
   writei(f, 'houdini.help_url STRING "%s"' % sd.help_url ,1)

   WriteHoudiniHeader(sd, f)

   
def WriteMTDParam(f, name, ptype, value, d):
   if value == None:
      return

   if ptype == 'bool':
      if value:
         bval = "TRUE"
      else:
         bval = "FALSE"
      writei(f, '%s BOOL %s' % (name, bval), d)
   elif ptype == 'int':
      writei(f, '%s INT %d' % (name, value), d)
   elif ptype == 'float':
      writei(f, '%s FLOAT %r' % (name, value), d)
   elif ptype == 'string':
      writei(f, '%s STRING "%s"' % (name, value), d)

def WriteMTD(sd, fn):      
   
   #build tab and group lists
   for e in sd.root.children:
      buildGroupList(sd, e)
   
   #make all groupident unique
   sd.uniqueGroupIdents()  

   f = open(fn, 'w') 
   
   WriteMDTHeader(sd, f)   

   writei(f, '')

   for p in sd.parameters:
      writei(f, '', 1)
      writei(f, '[attr %s]' % p.name, 1)
      writei(f, 'houdini.label STRING "%s"' % p.label, 2)
      WriteMTDParam(f, "min", "float", p.mn, 2)
      WriteMTDParam(f, "max", "float", p.mx, 2)
      WriteMTDParam(f, "softmin", "float", p.smn, 2)
      WriteMTDParam(f, "softmax", "float", p.smx, 2)               
      WriteMTDParam(f, "desc", "string", p.description, 2)
      WriteMTDParam(f, "houdini.help", "string", p.description, 2)
      WriteMTDParam(f, "linkable", "bool", p.connectible, 2)
      if p.ui == 'file':
         WriteMTDParam(f, "c4d.gui_type", "int", 3, 2)
         WriteMTDParam(f, "houdini.type", "string", "file:image", 2)

      if p.houdini_disable_when:
         WriteMTDParam(f, "houdini.disable_when", "string", p.houdini_disable_when, 2)

      if p.houdini_join_next:
         WriteMTDParam(f, "houdini.join_next", "bool", p.houdini_join_next, 2)
      
      if p.houdini_default:
         WriteMTDParam(f, "houdini.default", "enum", p.houdini_default,2)
      
      if p.maya_hide:
         WriteMTDParam(f, "maya.hide", "bool", p.maya_hide, 2)

      if p.filePathBrowse:
         WriteMTDParam(f, "c4d.gui_type", "int", 3, 2)
         WriteMTDParam(f, "c4d.label", "string", "File Path", 2)
         WriteMTDParam(f, "maya.usedAsFilename", "bool", True, 2)
         WriteMTDParam(f, "c4d.gui_type", "int", 3, 2)
         WriteMTDParam(f, "houdini.type", "string", "file:image", 2)
         WriteMTDParam(f, "houdini.callback", "string", "python:import htoa.texture; htoa.texture.imageFilenameCallback()", 2)

   for a in sd.aovs:
      writei(f, '[attr %s]' % a.name, 1)
      writei(f, 'houdini.label STRING "%s"' % a.label, 2)
      if (a.ptype == 'rgb'):
         writei(f, 'aov.type INT 0x05', 2)
      elif (a.ptype == 'rgba'):
         writei(f, 'aov.type INT 0x06', 2)
      writei(f, 'aov.enable_composition BOOL TRUE', 2)               
      writei(f, 'default STRING "%s"' % a.name[4:], 2)

   f.close()

def WalkArgs(el, f, d):
   if isinstance(el, Group):
      writei(f, '<page name="%s">' % (el.name), d)

      if el.children:
         for e in el.children:
            WalkArgs(e, f, d+1)

      writei(f, '</page>', d)
   elif isinstance(el, AOV):
      writei(f, '<param name="%s" label="%s"/>' % (el.name, el.label), d)
   elif isinstance(el, Parameter):
      if el.ptype == 'bool':
         writei(f, '<param name="%s" label="%s" widget="checkBox"/>' % (el.name, el.label), d)
      elif el.ptype == 'int':
         writei(f, '<param name="%s" label="%s" int="True"' % (el.name, el.label), d)
         if el.mn:
            writei(f, 'min="%s"' % el.mn, d)
         writei(f, '/>', d)
      elif el.ptype == 'rgb':
         writei(f, '<param name="%s" label="%s" widget="color"/>' % (el.name, el.label), d)
      elif el.ptype == 'enum':
         writei(f, '<param name="%s" label="%s" widget="popup">' % (el.name, el.label), d)
         
         writei(f, '<hintdict name="options">', d+1)
         
         for en in el.enum_names:
            writei(f, '<string value="%s"/>' % en, d+2)  

         writei(f, '</hintdict>', d+1)
         writei(f, '</param>', d)
      else:
         writei(f, '<param name="%s" label="%s"/>' % (el.name, el.label), d)


def WriteArgs(sd, fn):
   f = open(fn, 'w')
   writei(f, '<args format="1.0">', 0)
   writei(f, '<param widget="null" label="Exposure" slider="True" slidermin="-100.0" min="0.0 "slidermax="100.0" max="100.0"/>', 0)
   writei(f, '<param widget="null" label="Rolling Shutter" slider="True"/>', 0)
   writei(f, '<param widget="null" label="Filtermap"/>', 0)
   writei(f, '<param widget="null" name="projective"/>', 0)
   writei(f, '<param widget="null" name="aperture_blades" label="Aperture Blades" slider="True" slidermin="0" slidermax="40"/>', 0)
   writei(f, '<param widget="null" label="Aperture Rotation" slider="True" slidermin="0.0" min="0.0" slidermax="50.0" max="360.0"/>', 0)
   writei(f, '<param widget="null" label="Aperture Blade Curvature" slider="True" slidermin="-20.0" min="-20.0" slidermax="20.0" max="20.0"/>', 0)
   writei(f, '<param widget="null" label="Aperture Aspect Ratio" slider="True"/>', 0)
   writei(f, '<param widget="null" name="handedness">', 0)
   writei(f, '<hintdict name="options">', 1)
   writei(f, '<string value="right"/>', 1)
   writei(f, '<string value="left"/>', 1)
   writei(f, '</hintdict>', 1)
   writei(f, '</param>', 1)
   writei(f, '<param widget="null"/>', 0)
   writei(f, '<param widget="null"/>', 0)
   writei(f, '<param widget="null" label="Shutter Start" slider="True"/>', 0)
   writei(f, '<param widget="null" label="Shutter End" slider="True"/>', 0)
   writei(f, '<param widget="null" label="Motion Start" slider="True"/>', 0)
   writei(f, '<param widget="null" label="Motion End" slider="True"/>', 0)
   writei(f, '<param widget="null" name="shutter_type">', 0)
   writei(f, '<hintdict name="options">', 1)
   writei(f, '<string value="box"/>', 1)
   writei(f, '<string value="curve"/>', 1)
   writei(f, '<string value="triangle"/>', 1)
   writei(f, '</hintdict>', 1)
   writei(f, '</param>', 1)
   writei(f, '<param widget="null" name="rolling_shutter" label="Rolling Shutter">', 0)
   writei(f, '<hintdict name="options">', 1)
   writei(f, '<string value="top"/>', 1)
   writei(f, '<string value="left"/>', 1)
   writei(f, '<string value="off"/>', 1)
   writei(f, '<string value="right"/>', 1)
   writei(f, '<string value="bottom"/>', 1)
   writei(f, '</hintdict>', 1)
   writei(f, '</param>', 1)
   writei(f, '<param widget="null" isDynamicArray="1" name="horizontal_fov"/>', 0)
   writei(f, '<param widget="null" isDynamicArray="1" name="vertical_fov"/>', 0)
   writei(f, '<param widget="null" isDynamicArray="1" name="screen_window_min"/>', 0)
   writei(f, '<param widget="null" isDynamicArray="1" name="screen_window_max"/>   ', 0)
   writei(f, '<param widget="null" isDynamicArray="1" name="shutter_curve"/>   ', 0)
   writei(f, '<param widget="null" isDynamicArray="1" name="position"/>', 0)
   writei(f, '<param widget="null" isDynamicArray="1" name="look_at"/>', 0)
   writei(f, '<param widget="null" isDynamicArray="1" name="up"/>', 0)
   writei(f, '<param tupleGroupSize="4" widget="null" isDynamicArray="1" name="matrix"/>', 0)
   writei(f, '<param name="near_clip" widget="null"/>', 0)
   writei(f, '<param name="far_clip" widget="null"/>', 0)
   writei(f, '<param name="shutter_start" widget="null"/>   ', 0)
   writei(f, '<param name="shutter_end" widget="null"/>', 0)
   writei(f, '<param name="rolling_shutter_duration" widget="null"/>   ', 0)
   writei(f, '<param name="motion_start" widget="null"/>   ', 0)
   writei(f, '<param name="motion_end" widget="null"/>', 0)
   writei(f, '<param name="exposure" widget="null"/>   ', 0)
   writei(f, '<param name="filtermap" widget="null"/>   ', 0)

   for e in sd.root.children:
      WalkArgs(e, f, 0)

   writei(f, '</args>', 0)


def remapControls(sd):
   with group(sd, 'Remap', collapse=True, description='These controls allow you to remap the shader result.'):
      sd.parameter('RMPinputMin', 'float', 0.0, label='Input min', description='Sets the minimum input value. Use this to pull values outside of 0-1 into a 0-1 range.')
      sd.parameter('RMPinputMax', 'float', 1.0, label='Input max', description='Sets the maximum input value. Use this to pull values outside of 0-1 into a 0-1 range.')

      with group(sd, 'Contrast', collapse=False):
         sd.parameter('RMPcontrast', 'float', 1.0, label='Contrast', description='Scales the contrast of the input signal.')
         sd.parameter('RMPcontrastPivot', 'float', 0.18, label='Pivot', description='Sets the pivot point around which the input signal is contrasted.')

      with group(sd, 'Bias and gain', collapse=False):
         sd.parameter('RMPbias', 'float', 0.5, label='Bias', description='Bias the signal higher or lower. Values less than 0.5 push the average lower, values higher than 0.5 push it higher.')
         sd.parameter('RMPgain', 'float', 0.5, label='Gain', description='Adds gain to the signal, in effect a different form of contrast. Values less than 0.5 increase the gain, values greater than 0.5 decrease it.')

      sd.parameter('RMPoutputMin', 'float', 0.0, label='Output min', description='Sets the minimum value of the output. Use this to scale a 0-1 signal to a new range.')
      sd.parameter('RMPoutputMax', 'float', 1.0, label='Output max', description='Sets the maximum value of the output. Use this to scale a 0-1 signal to a new range.')

      with group(sd, 'Clamp', collapse=False):
         sd.parameter('RMPclampEnable', 'bool', False, label='Enable', description='When enabled, will clamp the output to Min-Max.')
         sd.parameter('RMPthreshold', 'bool', False, label='Expand', description='When enabled, will expand the clamped range to 0-1 after clamping.')
         sd.parameter('RMPclampMin', 'float', 0.0, label='Min', description='Minimum value to clamp to.')
         sd.parameter('RMPclampMax', 'float', 1.0, label='Max', description='Maximum value to clamp to.')


# Main. Load the UI file and build UI templates from the returned structure
if __name__ == '__main__':
   if len(sys.argv) < 4:
      print('ERROR: must supply exactly ui source input and mtd, ae, spdl and args outputs')
      sys.exit(1)

   ui = ShaderDef()
   globals_dict = {'ui':ui}
   with open(sys.argv[1]) as f:
    code = compile(f.read(), sys.argv[1], 'exec')
    exec(code,  globals_dict)

   if not isinstance(ui, ShaderDef):
      print('ERROR: ui object is not a ShaderDef. Did you assign something else to it by mistake?')
      sys.exit(2)

   WriteMTD(ui, sys.argv[2])  
   WriteAETemplate(ui, sys.argv[3])
   #WriteAEXML(ui, sys.argv[4])
   #WriteNEXML(ui, sys.argv[5])
   #WriteSPDL(ui, sys.argv[6])
   WriteArgs(ui, sys.argv[4])

   # C4DtoA resource files
   name = os.path.basename(os.path.splitext(sys.argv[1])[0])
   build_dir = sys.argv[8] if len(sys.argv) > 6 else os.path.abspath("")
   WriteC4DtoAResourceFiles(ui, name, build_dir)
