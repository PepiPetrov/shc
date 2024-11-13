import os

def remove_all_files_in_directory(directory):
    try:
        # Check if the specified path is a directory
        if os.path.isdir(directory):
            # Loop through the items in the directory
            for filename in os.listdir(directory):
                # Construct the full file path
                file_path = os.path.join(directory, filename)
                try:
                    # Check if the item is a file and remove it
                    if os.path.isfile(file_path):
                        os.remove(file_path)
                        print(f"Removed file: {file_path}")
                    # You can optionally remove subdirectories if needed
                    # elif os.path.isdir(file_path):
                    #     os.rmdir(file_path)
                except Exception as e:
                    print(f"Error deleting {file_path}. Reason: {str(e)}")
        else:
            print(f"{directory} is not a valid directory.")
    except Exception as e:
        print(f"Error accessing directory {directory}. Reason: {str(e)}")

# Specify the directory you want to clean
directory = "bin/obj"

# Remove all files in the specified directory
remove_all_files_in_directory(directory)
