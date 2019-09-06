import mtoa.ui.ae.templates as templates
import pymel.core as pm
import maya.cmds as cmds
import mtoa.ui.ae.utils as aeUtils

class aiLentilTemplate(templates.AttributeTemplate):
    """
    def filenameEditBokeh(self, mData) :
        attr = self.nodeAttr('aiBokehEXRPath')
        cmds.setAttr(attr,mData,type="string")

    def LoadFilenameButtonPushBokeh(self, *args):
        basicFilter = 'All Files (*.*)'
        ret = cmds.fileDialog2(fileFilter=basicFilter, dialogStyle=2, cap='Select sample_bokeh file location',fm=0)
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

    """


    def setup(self):

        self.beginLayout("Polynomial Optics", collapse=False)
        self.addControl("aiUnitModel", label="Units")
        self.addControl("aiLensModel", label="Lens Model")
        self.addControl("aiSensorWidth", label="Sensor Width (mm)")
        self.addControl("aiWavelength", label="Wavelength (nm)")
        self.addControl("aiDof", label="Enable depth of field")
        self.addControl("aiFstop", label="F-stop", dynamic=True)
        self.addControl("aiFocalDistance", label="Focus distance (unit)")
        self.addControl("aiExtraSensorShift", label="Extra Sensor shift (mm)")
        self.addControl("aiVignettingRetries", label="Vignetting retries")
        self.addControl("aiApertureBlades", label="Aperture blades")
        self.addControl("aiProperRayDerivatives", label="Proper Ray Derivatives")
        self.addControl("aiUseImage", label="Use Bokeh Image")
        self.addControl("aiBokehInputPath", label="Bokeh Path")
        self.addControl("aiEmpiricalCaDist", label="Empirical Chromatic Aberration Distance")
        
        # add these in the aovshader template instead
        # self.suppress('normalCamera') 
        # self.suppress('hardwareColor')
        
        
        self.endLayout()

        
        """
        self.beginLayout("AOV shader", collapse=False)
        self.addControl("aiBackwardSamples", label="Backwards samples")
        self.addControl("aiMinimumRgb", label="Minimum RGB")
        self.addCustom("aiBokehEXRPath", self.filenameNewBokeh, self.filenameReplaceBokeh)
        self.endLayout()
        """



templates.registerTranslatorUI(aiLentilTemplate, "camera", "lentil")