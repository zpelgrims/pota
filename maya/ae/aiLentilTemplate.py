import mtoa.ui.ae.templates as templates
import pymel.core as pm
import maya.cmds as cmds
import mtoa.ui.ae.utils as aeUtils

class aiLentilTemplate(templates.AttributeTemplate):

    def filenameEditBokehOutput(self, mData) :
        attr = self.nodeAttr('aiBokehExrPath')
        cmds.setAttr(attr,mData,type="string")

    def LoadFilenameButtonPushBokehOutput(self, *args):
        basicFilter = 'All Files (*.*)'
        ret = cmds.fileDialog2(fileFilter=basicFilter, dialogStyle=2, cap='Select file location',fm=0)
        if ret is not None and len(ret):
            self.filenameEditBokehOutput(ret[0])
            cmds.textFieldButtonGrp("filenameBokehGrpOutput", edit=True, text=ret[0])

    def filenameNewBokehOutput(self, nodeName):
        path = cmds.textFieldButtonGrp("filenameBokehGrpOutput", label="Bidirectional output", changeCommand=self.filenameEditBokehOutput, width=300)
        cmds.textFieldButtonGrp(path, edit=True, text=cmds.getAttr(nodeName))
        cmds.textFieldButtonGrp(path, edit=True, buttonLabel="...", buttonCommand=self.LoadFilenameButtonPushBokehOutput)

    def filenameReplaceBokehOutput(self, nodeName):
        cmds.textFieldButtonGrp("filenameBokehGrpOutput", edit=True, text=cmds.getAttr(nodeName) )

    
    def filenameEditBokehInput(self, mData) :
        attr = self.nodeAttr('aiBokehInputPath')
        cmds.setAttr(attr,mData,type="string")

    def LoadFilenameButtonPushBokehInput(self, *args):
        basicFilter = 'All Files (*.*)'
        ret = cmds.fileDialog2(fileFilter=basicFilter, dialogStyle=2, cap='Select file location',fm=0)
        if ret is not None and len(ret):
            self.filenameEditBokehInput(ret[0])
            cmds.textFieldButtonGrp("filenameBokehGrpInput", edit=True, text=ret[0])

    def filenameNewBokehInput(self, nodeName):
        path = cmds.textFieldButtonGrp("filenameBokehGrpInput", label="Bokeh Image", changeCommand=self.filenameEditBokehInput, width=300)
        cmds.textFieldButtonGrp(path, edit=True, text=cmds.getAttr(nodeName))
        cmds.textFieldButtonGrp(path, edit=True, buttonLabel="...", buttonCommand=self.LoadFilenameButtonPushBokehInput)

    def filenameReplaceBokehInput(self, nodeName):
        cmds.textFieldButtonGrp("filenameBokehGrpInput", edit=True, text=cmds.getAttr(nodeName) )




    def setup(self):

        self.beginLayout("Polynomial Optics", collapse=False)
        self.addControl("aiLensModelPO", label="Lens Model")
        self.addControl("aiSensorWidthPO", label="Sensor Width (mm)")
        self.addControl("aiWavelengthPO", label="Wavelength (nm)")
        self.addControl("aiDofPO", label="Enable depth of field")
        self.addControl("aiFstopPO", label="F-stop", dynamic=True)
        self.addControl("aiFocusDistancePO", label="Focus distance (unit)")
        self.addControl("aiExtraSensorShiftPO", label="Extra Sensor shift (mm)")
        self.addControl("aiApertureBladesPO", label="Aperture blades")
        self.endLayout()

        self.beginLayout("Bidirectional")
        self.addCustom("aiBokehExrPathPO", self.filenameNewBokehOutput, self.filenameReplaceBokehOutput)
        self.addControl("aiBokehSamplesMultPO", label="Sampling multiplier")
        self.addControl("aiMinimumRgbPO", label="Min bidir luminance")
        self.addControl("aiAdditionalLuminancePO", label="Additional Luminance")
        self.addControl("aiLuminanceRemapTransitionWidthPO", label="Add lum trans width")
        self.endLayout()

        self.beginLayout("Bokeh Image")
        self.addControl("aiUseImagePO", label="Use Bokeh Image")
        self.addCustom("aiBokehInputPathPO", self.filenameNewBokehInput, self.filenameReplaceBokehInput)

        self.endLayout()
        

        self.beginLayout("Advanced options")
        self.addControl("aiUnitsPO", label="Units")
        self.addControl("aiProperRayDerivativesPO", label="Proper Ray Derivatives")
        self.addControl("aiVignettingRetriesPO", label="Vignetting retries")
        self.endLayout()


templates.registerTranslatorUI(aiLentilTemplate, "camera", "lentil")