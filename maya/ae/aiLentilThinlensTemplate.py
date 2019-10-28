import mtoa.ui.ae.templates as templates
import pymel.core as pm
import maya.cmds as cmds
import mtoa.ui.ae.utils as aeUtils

class aiLentilThinlensTemplate(templates.AttributeTemplate):

    def filenameEditBokehOutput(self, mData) :
        attr = self.nodeAttr('aiBokehExrPathTL')
        cmds.setAttr(attr, mData,type="string")

    def LoadFilenameButtonPushBokehOutput(self, *args):
        basicFilter = 'All Files (*.*)'
        ret = cmds.fileDialog2(fileFilter=basicFilter, dialogStyle=2, cap='Select file location',fm=0)
        if ret is not None and len(ret):
            self.filenameEditBokehOutput(ret[0])
            cmds.textFieldButtonGrp("filenameBokehGrpOutputTL", edit=True, text=ret[0])

    def filenameNewBokehOutput(self, nodeName):
        path = cmds.textFieldButtonGrp("filenameBokehGrpOutputTL", label="Bidirectional output", changeCommand=self.filenameEditBokehOutput, width=300)
        cmds.textFieldButtonGrp(path, edit=True, text=cmds.getAttr(nodeName))
        cmds.textFieldButtonGrp(path, edit=True, buttonLabel="...", buttonCommand=self.LoadFilenameButtonPushBokehOutput)

    def filenameReplaceBokehOutput(self, nodeName):
        cmds.textFieldButtonGrp("filenameBokehGrpOutputTL", edit=True, text=cmds.getAttr(nodeName) )

    
    def filenameEditBokehInput(self, mData) :
        attr = self.nodeAttr('aiBokehInputPathTL')
        cmds.setAttr(attr,mData,type="string")

    def LoadFilenameButtonPushBokehInput(self, *args):
        basicFilter = 'All Files (*.*)'
        ret = cmds.fileDialog2(fileFilter=basicFilter, dialogStyle=2, cap='Select file location',fm=0)
        if ret is not None and len(ret):
            self.filenameEditBokehInput(ret[0])
            cmds.textFieldButtonGrp("filenameBokehGrpInputTL", edit=True, text=ret[0])

    def filenameNewBokehInput(self, nodeName):
        path = cmds.textFieldButtonGrp("filenameBokehGrpInputTL", label="Bokeh Image", changeCommand=self.filenameEditBokehInput, width=300)
        cmds.textFieldButtonGrp(path, edit=True, text=cmds.getAttr(nodeName))
        cmds.textFieldButtonGrp(path, edit=True, buttonLabel="...", buttonCommand=self.LoadFilenameButtonPushBokehInput)

    def filenameReplaceBokehInput(self, nodeName):
        cmds.textFieldButtonGrp("filenameBokehGrpInputTL", edit=True, text=cmds.getAttr(nodeName) )



    def setup(self):

        self.beginLayout("Thin Lens", collapse=False)
        self.addControl("aiSensorWidthTL", label="Sensor Width (cm)")
        self.addControl("aiFocalLengthTL", label="Focal Length (cm)")
        self.addControl("aiFstopTL", label="F-stop", dynamic=True)
        self.addControl("aiFocusDistanceTL", label="Focus distance (cm)")

        self.addControl("aiOpticalVignettingDistanceTL", label="Optical Vignetting Distance")
        self.addControl("aiOpticalVignettingRadiusTL", label="Optical Vignetting Radius")
        self.addControl("aiBiasTL", label="Bias")
        
        self.addControl("aiSquareTL", label="Circle to Square mapping")
        self.addControl("aiSqueezeTL", label="Anamorphic stretch")

        self.addControl("aiEmpericalCaDistTL", label="Chromatic Aberration")

        self.endLayout()

        
        self.beginLayout("Bidirectional", collapse=False)
        self.addControl("aiMinimumRgbTL", label="Bidirectional Trigger")
        self.addControl("aiBokehSamplesMultTL", label="Samples")
        self.addCustom("aiBokehExrPathTL", self.filenameNewBokehOutput, self.filenameReplaceBokehOutput)
        self.addControl("aiAdditionalLuminanceTL", label="Add bokeh luminance")
        self.addControl("aiLuminanceRemapTransitionWidthTL", label="Add Lum transition")
        self.endLayout()
        
        self.beginLayout("Bokeh Image")
        self.addControl("aiUseImageTL", label="Use Bokeh Image")
        self.addCustom("aiBokehInputPathTL", self.filenameNewBokehInput, self.filenameReplaceBokehInput)
        self.endLayout()


templates.registerTranslatorUI(aiLentilThinlensTemplate, "camera", "lentil_thinlens")