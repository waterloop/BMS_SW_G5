__doc__ = """
	To run:
		cd /path/to/this/file
		python3 generate_lut.py

	Generates a lookup table in your current working directory.
	Edit the LUT_NAME, LUT_FUNC, and LUT_SIZE variables	to generate the LUT that you need.

	Note: the size of the LUT in bytes will be 4*LUT_SIZE
"""

import numpy as np

COL_SIZE = 8

LUT_NAME = "ADC_TO_VOLTAGE_LUT"
LUT_SIZE = 1 << 12
def LUT_FUNC(x):
	return x*(3.3/(1 << 12))

def main():
	lut = [LUT_FUNC(x) for x in range(LUT_SIZE)]

	with open(f"{LUT_NAME}.c", "w") as f:
		f.write(f"const float {LUT_NAME}[{LUT_SIZE}] = {{\n\t")

		for i, lut_i in enumerate(lut):
			f.write(f"{lut_i:.7f}")		# floats in C have max 7 decimal places of precision

			if i != (len(lut) - 1):
				f.write(", ")

			if ((COL_SIZE - 1) - i) % COL_SIZE == 0:
				f.write("\n\t")

		f.write("\n};")

if __name__ == "__main__":
	main()
