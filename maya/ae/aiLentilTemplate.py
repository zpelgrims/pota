import mtoa.ui.ae.templates as templates
import pymel.core as pm
import maya.cmds as cmds
import mtoa.ui.ae.utils as aeUtils

class aiLentilTemplate(templates.AttributeTemplate):

    def filenameEditBokehInput(self, mData) :
        attr = self.nodeAttr('bokehImagePathPO')
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

    def launchgui_button_create(self, *args):
        # userinterface.py needs to be in the PYTHONPATH envvar!
        cmds.button(label="Launch Lentil UI", align="center", command="import userinterface as lentil_ui;reload(lentil_ui);lentil_ui.launch_maya()")

    def launchgui_button_update(self, *args):
        pass


    def setup(self):

        #self.addCustom("launchgui_button", self.launchgui_button_create, self.launchgui_button_update)

        self.beginLayout("Polynomial Optics", collapse=False)
        self.addControl("lensModelPO", label="Lens Model")
        self.addControl("sensorWidthPO", label="Sensor Width (mm)")
        self.addControl("wavelengthPO", label="Wavelength (nm)")
        self.addControl("dofPO", label="Enable depth of field")
        self.addControl("fstopPO", label="F-stop", dynamic=True)
        self.addControl("focusDistancePO", label="Focus distance (cm)")
        self.addControl("extraSensorShiftPO", label="Extra Sensor shift (mm)")
        self.addControl("bokehApertureBladesPO", label="Aperture blades")
        self.endLayout()

        self.beginLayout("Bidirectional")
        self.addControl("bidirSampleMultPO", label="Samples")
        self.addControl("bidirMinLuminancePO", label="Minimum luminance")
        self.addControl("bidirAddLuminancePO", label="Additional Luminance")
        self.addControl("bidirAddLuminanceTransitionPO", label="Add lum trans width")
        self.endLayout()

        self.beginLayout("Bokeh Image")
        self.addControl("bokehEnableImagePO", label="Use Bokeh Image")
        self.addCustom("bokehImagePathPO", self.filenameNewBokehInput, self.filenameReplaceBokehInput)

        self.endLayout()
        

        self.beginLayout("Advanced options")
        self.addControl("vignettingRetriesPO", label="Vignetting retries")
        self.addControl("unitsPO", label="Units")

        self.endLayout()


templates.registerTranslatorUI(aiLentilTemplate, "camera", "lentil")