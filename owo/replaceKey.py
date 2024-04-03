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
    modified_content = replace_text_with_filename(content, filename, counter)

    y = content.split('~')
    x = modified_content.split('~')
    diff = False
    for a,b in zip(x[1:], y[1:]):
      if a != b:
        diff = True
    counter += 1
    if diff:
      print(f'Rewritten in {filename} \n {"".join(content[:2]) }  \n {"".join(modified_content[:2])}')
    with open(filename, "w") as file:
      file.write(modified_content)

        
# Get filenames with .tact extension
filenames = glob.glob("*.owo")

# Rewrite files with modified content
rewrite_files(filenames)

print("Files rewritten successfully!")