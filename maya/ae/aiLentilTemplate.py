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
        self.addControl("aiLensModel", label="Lens Model")
        self.addControl("aiSensorWidth", label="Sensor Width (mm)")
        self.addControl("aiWavelength", label="Wavelength (nm)")
        self.addControl("aiDof", label="Enable depth of field")
        self.addControl("aiFstop", label="F-stop", dynamic=True)
        self.addControl("aiFocusDistance", label="Focus distance (unit)")
        self.addControl("aiExtraSensorShift", label="Extra Sensor shift (mm)")
        self.addControl("aiVignettingRetries", label="Vignetting retries")
        self.addControl("aiApertureBlades", label="Aperture blades")
        self.addControl("aiEmpiricalCaDist", label="Chromatic Aberration Distance")
        self.endLayout()

        self.beginLayout("Bidirectional")
        self.addCustom("aiBokehExrPath", self.filenameNewBokehOutput, self.filenameReplaceBokehOutput)
        self.addControl("aiBackwardSamples", label="Extra backward samples")
        self.addControl("aiMinimumRgb", label="Min bidirectional")
        self.endLayout()

        self.beginLayout("Bokeh Image")
        self.addControl("aiUseImage", label="Use Bokeh Image")
        self.addCustom("aiBokehInputPath", self.filenameNewBokehInput, self.filenameReplaceBokehInput)

        self.endLayout()
        

        self.beginLayout("Advanced options")
        self.addControl("aiUnitModel", label="Units")
        self.addControl("aiProperRayDerivatives", label="Proper Ray Derivatives")
        self.endLayout()


templates.registerTranslatorUI(aiLentilTemplate, "camera", "lentil")