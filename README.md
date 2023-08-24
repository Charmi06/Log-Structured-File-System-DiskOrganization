# Log Structured File System: Disk Organization
Implementation of a log-structured file system imitation with a focus on disk organization where the user will be able to get the text file from a Linux or Mac system and will be able to read, update, and delete the file from the log-structured file system. Here buffer cache will be handled by the host operating system memory management.

Implementation Instruction: 
implementation can be performed on both macOS and LINUX, I would recommend before executing the commands, please use the below steps.

1. make clean – This will clean the make file which was stored before in any system.
   ```bash
   make clean
   <button id="copy-button-1">Copy</button>
   <script>
      setupCopyButton('copy-button-1', 'pre code');
   </script>


   <script>
function setupCopyButton(buttonId, codeBlockSelector) {
  const copyButton = document.getElementById(buttonId);
  const commandCodeBlock = document.querySelector(codeBlockSelector);

  copyButton.addEventListener('click', () => {
    const commandText = commandCodeBlock.textContent.trim();
    const tempTextarea = document.createElement('textarea');
    tempTextarea.value = commandText;
    document.body.appendChild(tempTextarea);
    tempTextarea.select();
    document.execCommand('copy');
    document.body.removeChild(tempTextarea);
    copyButton.textContent = 'Copied!';
    setTimeout(() => {
      copyButton.textContent = 'Copy';
    }, 1500);
  });
}
</script>

