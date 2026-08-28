import zipfile
import os

zip_path = "/home/aizen-sosuke/iso/OniOS/doom-box.zip"
out_dir = "/home/aizen-sosuke/iso/OniOS/doom_files"

os.makedirs(out_dir, exist_ok=True)

with zipfile.ZipFile(zip_path, 'r') as zip_ref:
    zip_ref.extractall(out_dir)
    print("Files in zip:")
    for name in zip_ref.namelist():
        print(" -", name)
