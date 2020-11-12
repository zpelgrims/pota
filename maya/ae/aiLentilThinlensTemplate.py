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
        self.addControl("sensorWidth", label="Sensor Width (mm)")
        self.addControl("focalLength", label="Focal Length (mm)")
        self.addControl("fstop", label="F-stop", dynamic=True)
        self.addControl("focusDistance", label="Focus distance (cm)")
        self.addControl("enableDof", label="Enable DOF")

        self.addControl("opticalVignettingDistance", label="Optical Vignetting Distance")
        self.addControl("opticalVignettingRadius", label="Optical Vignetting Radius")
        self.addControl("abbSpherical", label="Aberration (spherical)")
        self.addControl("abbDistortion", label="Aberration (distortion)")
        self.addControl("bokehApertureBlades", label="Aperture Blades")
        self.addControl("bokehCircleToSquare", label="Circle to Square mapping")
        self.addControl("bokehAnamorphic", label="Anamorphic stretch")
        self.endLayout()


        self.beginLayout("Bokeh Image")
        self.addControl("bokehEnableImage", label="Enable Bokeh Image")
        self.addCustom("bokehImagePath", self.filenameNewBokehInput, self.filenameReplaceBokehInput)
        self.endLayout()

        
        self.beginLayout("Bidirectional", collapse=False)
        self.addControl("bidirMinLuminance", label="Bidirectional Trigger")
        self.addControl("bidirSampleMult", label="Samples")
        self.addControl("bidirAddLuminance", label="Add bokeh luminance")
        self.addControl("bidirAddLuminanceTransition", label="Add Lum transition")
        self.addControl("bidirDebug", label="Debug")
        self.endLayout()

        self.beginLayout("Advanced", collapse=False)
        self.addControl("vignettingRetries", label="Vignetting Retries")
        self.addControl("units", label="Units")
        self.endLayout()
        
        self.beginLayout("Experimental", collapse=False)
        self.addControl("abbComa", label="Aberration (coma)")
        self.endLayout()
        


templates.registerTranslatorUI(aiLentilThinlensTemplate, "camera", "lentil_thinlens")