import mtoa.ui.ae.templates as templates
import pymel.core as pm
import maya.cmds as cmds
import mtoa.ui.ae.utils as aeUtils

class aiPotaTemplate(templates.AttributeTemplate):

    def filenameEditBokeh(self, mData) :
        attr = self.nodeAttr('aiBokehEXRPath')
        cmds.setAttr(attr,mData,type="string")

    def LoadFilenameButtonPushBokeh(self, *args):
        basicFilter = 'All Files (*.*)'
        ret = cmds.fileDialog2(fileFilter=basicFilter, dialogStyle=2, cap='Load File',okc='Load',fm=4)
        if ret is not None and len(ret):
            self.filenameEditBokeh(ret[0])
            cmds.textFieldButtonGrp("filenameBokehGrp", edit=True, text=ret[0])

    def filenameNewBokeh(self, nodeName):
        path = cmds.textFieldButtonGrp("filenameBokehGrp", label="Bokeh AOV EXR path", changeCommand=self.filenameEditBokeh, width=300)
        cmds.textFieldButtonGrp(path, edit=True, text=cmds.getAttr(nodeName))
        cmds.textFieldButtonGrp(path, edit=True, buttonLabel="...",
        buttonCommand=self.LoadFilenameButtonPushBokeh)

    def filenameReplaceBokeh(self, nodeName):
        cmds.textFieldButtonGrp("filenameBokehGrp", edit=True, text=cmds.getAttr(nodeName) )


    def setup(self):
        self.beginLayout("Polynomial Optics", collapse=False)
        self.addControl("aiLensModel", label="Lens Model")
        self.addControl("aiSensorWidth", label="Sensor Width (mm)")
        self.addControl("aiWavelength", label="Wavelength (nm)")
        self.addControl("aiDof", label="Enable depth of field")
        self.addControl("aiFstop", label="F-stop")
        self.addControl("aiFocusDistance", label="Focus distance (cm)")
        self.addControl("aiExtraSensorShift", label="Extra Sensor shift (mm)")
        self.addControl("aiVignettingRetries", label="Vignetting retries")
        self.addControl("aiApertureBlades", label="Aperture blades")
        self.addControl("aiRunIntersectionTests", label="Run [0,0] intersection tests")
        
        self.endLayout()

        self.addSeparator()
        self.addSeparator()
        self.addSeparator()
        self.addSeparator()
        self.addSeparator()
        self.addSeparator()

        self.beginLayout("AOV shader", collapse=False)
        self.addControl("aiBackwardSamples", label="Backwards samples")
        self.addControl("aiMinimumRgb", label="Minimum RGB")
        self.addCustom("aiBokehEXRPath", self.filenameNewBokeh, self.filenameReplaceBokeh)
        self.endLayout()



templates.registerTranslatorUI(aiPotaTemplate, "camera", "pota")