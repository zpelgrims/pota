import mtoa.ui.ae.templates as templates
import pymel.core as pm
import maya.cmds as cmds
import mtoa.ui.ae.utils as aeUtils

class aiLentilThinlensTemplate(templates.AttributeTemplate):
    
    def filenameEditBokehInput(self, mData) :
        attr = self.nodeAttr('bokehImagePathTL')
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
        self.addControl("sensorWidthTL", label="Sensor Width (mm)")
        self.addControl("focalLengthTL", label="Focal Length (mm)")
        self.addControl("fstopTL", label="F-stop", dynamic=True)
        self.addControl("focusDistanceTL", label="Focus distance (cm)")
        self.addControl("enableDofTL", label="Enable DOF")

        self.addControl("opticalVignettingDistanceTL", label="Optical Vignetting Distance")
        self.addControl("opticalVignettingRadiusTL", label="Optical Vignetting Radius")
        self.addControl("abbSphericalTL", label="Aberration (spherical)")
        self.addControl("abbDistortionTL", label="Aberration (distortion)")
        self.addControl("bokehApertureBladesTL", label="Aperture Blades")
        self.addControl("bokehCircleToSquareTL", label="Circle to Square mapping")
        self.addControl("bokehAnamorphicTL", label="Anamorphic stretch")
        self.endLayout()


        self.beginLayout("Bokeh Image")
        self.addControl("bokehEnableImageTL", label="Enable Bokeh Image")
        self.addCustom("bokehImagePathTL", self.filenameNewBokehInput, self.filenameReplaceBokehInput)
        self.endLayout()

        
        self.beginLayout("Bidirectional", collapse=False)
        self.addControl("bidirMinLuminanceTL", label="Bidirectional Trigger")
        self.addControl("bidirSampleMultTL", label="Samples")
        self.addControl("bidirAddLuminanceTL", label="Add bokeh luminance")
        self.addControl("bidirAddLuminanceTransitionTL", label="Add Lum transition")
        self.endLayout()

        self.beginLayout("Advanced", collapse=False)
        self.addControl("vignettingRetriesTL", label="Vignetting Retries")
        self.addControl("unitsTL", label="Units")
        self.endLayout()
        
        self.beginLayout("Experimental", collapse=False)
        self.addControl("abbComaTL", label="Aberration (coma)")
        self.endLayout()
        


templates.registerTranslatorUI(aiLentilThinlensTemplate, "camera", "lentil_thinlens")