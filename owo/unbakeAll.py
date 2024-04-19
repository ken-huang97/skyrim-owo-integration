import re
import glob 

def replace_text_with_filename(file_content, filename, counter):
  """
  Replaces the text between the first set of ~ in the content with the filename without extension.

  Args:
      file_content (str): The content of the file.
      filename (str): The filename of the file.

  Returns:
      str: The modified content with the replaced text.
  """
  # Escape special characters in the filename for regular expression matching
  escaped_filename = re.escape(filename.split(".")[0])
  pattern = r"(\d+)~([^~]*)~"
  replacement = str(counter) + "~" + escaped_filename
  replacement = f'{counter}~{escaped_filename}~'
  return re.sub(pattern, replacement, file_content, count=1)

  
def unbake(input_string):
  """
  Unbakes a string following a specific format.

  Args:
      input_string (str): The string to unbake.

  Returns:
      str: The unbaked value (third part if at least four parts exist), 
          otherwise the original input string.
  """
  parts = input_string.split('~')
  if len(parts) >= 4:
    return parts[2]
  else:
    return input_string


def rewrite_files(filenames):
  """
  Iterates through filenames, reads content, replaces text with filename, 
  and rewrites the files with the modified content.

  Args:
      filenames (list): A list of filenames with the .tact extension.
  """
  counter = 0 
  for filename in filenames:
    with open(filename, "r") as file:
      content = file.read()
    modified_content = unbake(content)


    print(f'Rewritten in {filename} \n {content}  \n {modified_content}')
    # with open(filename, "w") as file:
    #   file.write(modified_content)

        
# Get filenames with .tact extension
filenames = glob.glob("*.owo")

# Rewrite files with modified content
rewrite_files(filenames)

print("Files rewritten successfully!")