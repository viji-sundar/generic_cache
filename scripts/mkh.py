import re
import sys

## Build the regex
regex    = r"\/[*]!proto[*]\/\s*(.*)\s*\/[*]!endproto[*]\/"
fileName = sys.argv[1]

## Open file passed as command line argument
text     = open( fileName, 'r' ).read()

## Do the match
matches  = re.finditer(regex, text, re.MULTILINE)

define   = '_' + fileName.replace('.', '_').upper()
print '#ifndef ' + define
print '#define ' + define + '\n'
for matchNum, match in enumerate(matches):
    print str(match.group(1)).strip() + ';'
print '\n#endif'
