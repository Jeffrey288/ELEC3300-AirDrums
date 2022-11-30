from os import listdir, getcwd, chdir
from os.path import isfile, join
# import librosa
from scipy.signal import resample
curpath = getcwd()
curpath = r"c:\Users\pinkp\Desktop\mod\mod2" # replace this with the path of the folder
# the folder should contain .wav sounds, MONOTONIC
chdir(curpath)
print(curpath)
onlyfiles = [f for f in listdir(curpath) if isfile(join(curpath, f))]
print(onlyfiles)
onlyfiles = [f for f in onlyfiles if f[-4:] == ".wav"]
print(onlyfiles)

f2=open("drumSounds.h", "w")
f2.write("#ifndef __DRUMSOUNDS_H\n#define __DRUMSOUNDS_H\n\n")
for fname in onlyfiles:
    f=open(fname, "rb") 
    header = f.read(44)
    freq = int.from_bytes(header[24:28], "little")
    tar_freq = 22050
    # print(freq)
    content = f.read()
    print(len(content))

    # x = [f'0x{int.from_bytes(content[i:i+1], "little"):02x}' for i in range(0, len(content))]
    # y = [str(int.from_bytes(content[i:i+1], "little")) for i in range(0, len(content))]
    y = [int.from_bytes(content[i:i+2], "little") for i in range(0, len(content), 2)]
    resample(y, int(len(y)/freq*22050))
    x = [f'0x{y[i]:04x}' for i in range(0, len(content)//2)]
    # DataFrame.resample()

    # x=y
    # print(max(y))

    f2.write(f"static const uint16_t {fname[:-4]}[{len(x)}] = " + "{\n    ")
    f2.write("\n    ".join([", ".join(x[10*i:10*i+10])+"," for i in range(len(x)//10)]))
    f2.write("\n};\n\n")
f2.write("#endif\n")
f.close()
f2.close()