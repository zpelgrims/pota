import mtoa.ui.ae.templates as templates
import maya.cmds as cmds
import mtoa.ui.ae.utils as aeUtils
# import maya.mel
from mtoa.ui.ae.shaderTemplate import ShaderAETemplate

class aiLentilCameraPolynomialOpticsTemplate(templates.AttributeTemplate):

    def filenameEditBokehInput(self, mData) :
        attr = self.nodeAttr('bokehImagePath')
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

        self.beginLayout("Global", collapse=False)
        # self.addControl("cameratype", label="Camera Type")
        self.addControl("sensorWidth", label="Sensor Width (mm)")
        self.addControl("enableDof", label="Enable depth of field")
        self.addControl("fstop", label="F-stop", dynamic=True)
        self.addControl("focusDist", label="Focus distance (cm)")
        self.endLayout()

        # po specific
        self.beginLayout("Polynomial Optics", collapse=False)
        self.addControl("lensModel", label="Lens Model")
        self.addControl("wavelength", label="Wavelength (nm)")
        self.addControl("extraSensorShift", label="Extra Sensor shift (mm)")
        self.endLayout()

        # tl specific
        # self.beginLayout("Thin Lens", collapse=False)
        # self.addControl("focalLengthLentil", label="Focal Length (mm)")
        # self.addControl("opticalVignettingDistance", label="Optical Vignetting Distance")
        # self.addControl("opticalVignettingRadius", label="Optical Vignetting Radius")
        # self.addControl("abbSpherical", label="Aberration (spherical)")
        # self.addControl("abbDistortion", label="Aberration (distortion)")
        # self.addControl("bokehCircleToSquare", label="Circle to Square mapping")
        # self.addControl("bokehAnamorphic", label="Anamorphic stretch")
        # self.beginLayout("Experimental", collapse=True)
        # self.addControl("abbComa", label="Aberration (coma)")
        # self.endLayout()
        # self.endLayout()

        # bidir
        self.beginLayout("Bidirectional")
        self.addControl("bidirSampleMult", label="Samples")
        self.addControl("bidirMinLuminance", label="Minimum luminance")
        self.addControl("bidirAddLuminance", label="Additional Luminance")
        self.addControl("bidirAddLuminanceTransition", label="Add lum trans width")
        self.endLayout()

        self.beginLayout("Bokeh Global")
        self.addControl("bokehApertureBlades", label="Aperture blades")
        self.addControl("bokehEnableImage", label="Use Bokeh Image")
        self.addCustom("bokehImagePath", self.filenameNewBokehInput, self.filenameReplaceBokehInput)
        self.endLayout()
        

        self.beginLayout("Advanced options")
        self.addControl("vignettingRetries", label="Vignetting retries")
        self.addControl("exp", label="Exposure")
        self.addControl("units", label="Units")
        self.endLayout()


templates.registerTranslatorUI(aiLentilCameraPolynomialOpticsTemplate, "camera", "lentil_camera_po")