import os 
import sys
from py_canoe import CANoe, wait

import argparse

# Initialize the parser
parser = argparse.ArgumentParser(description="A script to automate execution of tests in CANoe")

# Add arguments
parser.add_argument('--canoe_configuration', type=str, default=r"canoe_configuration\ACC.cfg", help="Canoe configuration path")

# Parse the arguments
args = parser.parse_args()

# seems that we should give absolute path to the canoe instance
canoe_configuration_path = rf"{os.getcwd()}\{args.canoe_configuration}"

print(f"Canoe configuration path:is {canoe_configuration_path}")

canoe_inst = CANoe()

canoe_inst.open(canoe_cfg=f"{canoe_configuration_path}")
canoe_inst.start_measurement()
wait(1)
verdict = canoe_inst.execute_test_module('ACC')
wait(1)
canoe_inst.stop_measurement()
wait(1)
with open(rf"{os.path.dirname(canoe_configuration_path)}\logging\canoe_write_window.txt", "w") as file:
    file.write(canoe_inst.application_com_obj.UI.Write.Text)
# work around to close the canoe configuration as it it seems to be not possible with COM interface ( as we need to clean up CI work space and it is not possible if configuration still open) 
# file:///C:/Program%20Files/Vector%20CANoe%20Family%2018/Help01/CANoeDEFamilyHTML5/CANoeDEFamily.htm#Topics/COMInterface/COMObjectHierarchy.htm?Highlight=COM%20interface%20hierarchy
canoe_inst.new()
exit_code = 0 if verdict == 1 else 1
sys.exit(exit_code)