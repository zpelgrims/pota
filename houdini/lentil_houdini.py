"""
* How to install

Copy lentil_houdini.py to HtoA's scripts folder.
$HTOA_PATH/scripts/python/htoa/lentil_houdini.py

It's necessary to inject an extra code into HtoA to be able to add a custom driver.
Therefore insert the following python patch after the line 71 of $HTOA_PATH/soho/arnold.py

try:
    from htoa.lentil_houdini import lentilPatch
    lentilPatch()
except StandardError:
    pass

"""

# Eternal gratitude to Vahan Sosoyan for figuring out the monkey patching in Aton (https://sosoyan.github.io/Aton/)

import hou
from htoa.node.parms import HaNodeSetStr
from htoa.node.node import nodeSetArrayString
from arnold import *


def warn(msg, *params):
    """ 
    Warn message in Arnold Rendering process
    @param msg: str
    @param params: __repr__
    @return:
    """
    header = "[%s] " % __name__
    AiMsgWarning(header + msg, *params)


def lentilPatch():
    """
    Patching HtoA to override the driver
    @return:
    """
    import htoa.object.rop
    import htoa.session

    # Only monkey patch once -- the arnold.py soho script from HtoA can and
    # typically will be called many times. Only monkey patch (decorate) the
    # generate method once.
    if htoa.object.rop.HaRop.generate.__name__ == "generate":
        htoa.session.HaRop.generate = generate_decorated(htoa.session.HaRop.generate)



def lentil_update(self):
    """ 
    Runs this function for overrides
    @param self: htoa.session.HaRop.generate
    @return: 
    """

    camnode = hou.node(self.session.camera_name)
    arnold_vopnet_camnode = camnode.parm("ar_camera_shader").eval() + "/OUT_camera"
    inputs = hou.node(arnold_vopnet_camnode).inputs()
    cam_shader_typename = inputs[0].type().name() # think i can always assume it's the first input?

    if cam_shader_typename == "arnold::lentil":
        driver = "lentil_bokeh_driver"
    elif cam_shader_typename == "arnold::lentil_thinlens":
        driver = "lentil_thin_lens_bokeh_driver"
    else:
        return
    
    if not (AiNodeEntryLookUp(driver) is None):

        lentil_driver_node = get_driver(self, driver)

        options_node = AiUniverseGetOptions(self.session.universe)

        # Get the outputs string array param (on the options node) as a python list
        array = AiNodeGetArray(options_node, "outputs")
        elements = AiArrayGetNumElements(array)
        outputs = [AiArrayGetStr(array, i) for i in xrange(elements)]
        if outputs:
            output_list = outputs[0].split()
            driver_name = output_list[-1]
            aton_name = AiNodeGetName(lentil_driver_node)

            aton_outputs = []
            for i in outputs:
                aton_outputs.append(i)
                if "variance_filter" not in i:
                    aton_outputs.append(i.replace(driver_name, aton_name))


            nodeSetArrayString(options_node, "outputs", aton_outputs)
    else:
        warn("Lentil Driver was not found.")


def generate_decorated(func):
    """ 
    Decorating a generate method
    @param func: htoa.session.HaRop.generate
    @return: function
    """
    def generate_decorator(self, *args, **kwargs):
        """ Extends generate method
        @param self: htoa.session.HaRop.generate
        @return: function
        """
        result = func(self, *args, **kwargs)
        lentil_update(self)
        return result

    return generate_decorator


def get_driver(self, node_entry_name):
    """
    Get driver Arnold node
    @param self: htoa.session.HaRop.generate
    @param node_entry_name: str
    @param new_sub_str: str
    @return: node of type driver
    """

    node_iter = AiUniverseGetNodeIterator(self.session.universe, AI_NODE_DRIVER)

    while not AiNodeIteratorFinished(node_iter):
        node = AiNodeIteratorGetNext(node_iter)
        node_entry = AiNodeGetNodeEntry(node)
        if node_entry_name == AiNodeEntryGetName(node_entry):
            return node

    return AiNode(self.session.universe, node_entry_name, "lentil_driver")