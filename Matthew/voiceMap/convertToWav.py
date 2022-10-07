import os

# files
src_dir = "newtrainingfiles/"
src_file = []

for root, dirs, files in os.walk(src_dir):
    for name in files:
        # print("name: ", name)
        src_file.append(src_dir + name)

# convert wav to mp3
for f in src_file:
    full_file = f
    print("full_file: ", full_file)

    split_word = "/"
    orig_str = f.split(split_word)[1]
    print("orig_str: ", orig_str)

    split_word = ".mov"
    res_str = orig_str.split(split_word)[0]
    print("res_str: ", res_str)

    name = res_str + ".wav"
    print("name: ", name, "\n")

    print("ffmpeg -i " + full_file + " " + src_dir + name)

    os.system("ffmpeg -i " + full_file + " " + src_dir + name)

