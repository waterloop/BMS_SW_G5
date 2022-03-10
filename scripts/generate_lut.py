__doc__ = """
    To run:
        cd /path/to/this/file
        python3 generate_lut.py

    Generates a lookup table in your current working directory.
    Edit the LUT_NAME, LUT_FUNC, and LUT_SIZE variables    to generate the LUT that you need.

    Note: the size of the LUT in bytes will be 4*LUT_SIZE
"""

import numpy as np

COL_SIZE = 8

LUT_NAME = "ADC_TO_TEMP_LUT"
LUT_SIZE = 1 << 12
def LUT_FUNC(x):
    polyfit_coeffs = np.array(
        [ 1.26667113e+02, -4.94221074e-02,  1.09519117e-05, -1.27781356e-09,
          8.44104863e-14, -3.36893986e-18,  8.44233082e-23, -1.34846038e-27,
          1.36025164e-32, -8.27619691e-38,  2.64031523e-43, -1.98711870e-49,
         -9.74006422e-55,  9.63875219e-61,  5.32852253e-66, -8.41079089e-72])

    def T(R):
        ret = 0
        for i, c in enumerate(polyfit_coeffs):
            ret += c*(R**i)
        return ret

    Vdd = 3.3
    Vo = x*(3.3/((1 << 12) - 1))

    try:
        R_thermistor = (1000*Vo)/(Vdd - Vo)
    except ZeroDivisionError:
        return 696969

    return T(R_thermistor)


def main():
    lut = [LUT_FUNC(x) for x in range(LUT_SIZE)]

    with open(f"{LUT_NAME}.c", "w") as f:
        f.write(f"const float {LUT_NAME}[{LUT_SIZE}] = {{\n\t")

        for i, lut_i in enumerate(lut):
            f.write(f"{lut_i:.7f}")    # floats in C have max 7 decimal places of precision

            if i != (len(lut) - 1):
                f.write(", ")

            if ((COL_SIZE - 1) - i) % COL_SIZE == 0:
                f.write("\n")
                if (i != (len(lut) - 1)):
                    f.write(" "*4)

        f.write("};")

if __name__ == "__main__":
    main()
