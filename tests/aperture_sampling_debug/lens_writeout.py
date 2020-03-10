import json

def read_lens(lens_json, fl):
    with open(lens_json) as data_file:    
        lens_database = json.load(data_file)
    
    fl_rt = lens_database["0001"]["focal-length-mm-raytraced"]
    fl_ratio = fl/fl_rt

    for element in lens_database["0001"]["optical-elements-adjusted"]:
        radius = element["radius"] * fl_ratio
        ior = element["ior"]
        thickness = element["thickness"] * fl_ratio
        abbe = element["abbe"]
        housingradius = element["housing-radius"] * fl_ratio

        print("[{}, {}, {}, {}, {}]".format(radius, thickness, ior, abbe, housingradius))

read_lens("/home/cactus/lentil/polynomial-optics/database/lenses.json", 49)