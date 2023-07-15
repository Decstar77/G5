import os

# get the current directory
current_dir = os.getcwd()

# iterate over all files in current directory
for filename in os.listdir(current_dir):
    # we only want to work with files, not directories
    if os.path.isfile(os.path.join(current_dir, filename)):
        # create the new filename: lower case and replace spaces with '_'
        new_filename = filename.lower().replace(' ', '_')
        
        # rename the file
        os.rename(os.path.join(current_dir, filename), os.path.join(current_dir, new_filename))
