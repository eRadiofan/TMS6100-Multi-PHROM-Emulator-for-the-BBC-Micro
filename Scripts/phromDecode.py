#
# Program for decoding TMS6100 phrase ROMs
#  and generating understandable C data for use in an emulator
#
# Radiofan, 2026
import sys

# Read all the data
with open(sys.argv[1], mode='rb') as file: # b is important -> binary
  fileContent = file.read()

fileLen = len(fileContent)

# Reverse bits in a byte
def reverse_Bits(n):
  result = 0;
  for i in range(8):
    result <<= 1
    result |= n & 1
    n >>= 1
  return bytes([result])

# Reverse bits in all the data
reversed = bytearray()
for x in range(fileLen):
  reversed += reverse_Bits(fileContent[x])

# Bit ordering in pointer bytes are the normal way around!
def get_pointer(a,b):
  return b*256 + a

# Get name character
def getNameChar(c):
  return bytes([c >> 1])

# Test for valid phrase label characters
def validNameChar(ch):
  c = ch >> 1
  if (c == ord('/')):
    return 0
  return (c >= ord('(') and c <= ord('9')) or (c >= ord('A') and c <= ord('Z')) or c == ord(' ')

# Data type to hold a phrase name in the Acorn format PHROM
class phraseNameEnt:
  def __init__(self, pointer, name, phrLen = 0):
    self.pointer = pointer
    self.name = name
    self.phrLen = phrLen

def ptrKey(obj):
  return obj.pointer

def uniq(ents):
  ri = 0
  res = []
  prevKey = 0
  for p in range(len(ents)):
    key = ents[p].pointer
    if (key == prevKey):
      res[ri-1].name += ", "+ents[p].name
    else:
      res.append(ents[p])
      ri += 1
    prevKey = key
  return res

# Bitwise functions and phrase parsing code
def bitsInit(start):
  global b_addr, b_data, b_count
  b_addr = start
  b_data = fileContent[start]
  b_count = 0
  #print("Byte:", f'0x{b_data:02x}', end=" ")

def getBit():
  global b_addr, b_data, b_count
  bit = (b_data & 1) > 0
  b_count += 1
  if (b_count == 8):
    if (b_addr >= fileLen):
      b_data = 0xFF
    else:
      b_addr += 1
      b_count = 0
      b_data = fileContent[b_addr]
      #print("Byte:", f'0x{b_data:02x}', end=" ")
  else:
    b_data = (b_data >> 1)
  return bit

def getBits(n):
  d = 0
  for i in range(n):
    d = (d << 1) | getBit()
  #print("getBits:", n, d)
  return d

# returns 1 if valid start frame, -1 of stop frame, 0 otherwise
def parseFrame(debug=False):
  energy = getBits(4)
  if (debug):
    print("// Energy:", f'0x{energy:1x}', end=" ")
  if (energy == 0):
    return 1
  if (energy == 0xF):
    return -1
  rept = getBits(1)
  pitch = getBits(6)
  if (debug):
    print("Repeat:", rept, "Pitch:", pitch, end=" ")
  if (rept):
    return 0
  K14 = getBits(18)
  if (debug):
    print("K1-K4:", f'0x{K14:04x}', end=" ")
  if (pitch != 0):
    K510 = getBits(21)
    if (debug):
      print("K5-K10:", f'0x{K510:04x}', end=" ")
  return 1

def validStartFrame(start):
  bitsInit(start)
  res = parseFrame(False)
  #print()
  return (res == 1)

def parsePhrase(start):
  global b_addr, b_count
  #print("Address:", f'{start:04x}')
  bitsInit(start)
  while (b_addr < fileLen-1):
    res = parseFrame()
    if (res == -1):
      break
  if (b_count == 0):
    return b_addr
  else:
    return b_addr + 1

# Output byte in hex format (original bit order)
def bout(addr):
  return f'0x{fileContent[addr]:02x}'

# Output data between indices, breaking line every 16 values,
#  with optional comment at start of block
def dout(start, end, comment=None):
  print(" ", end=" ")
  s = start
  while s < end:
    e = s + 16
    if (e > end):
      e = end
    if (s == start):
      print(' '.join('0x{:02x},'.format(x) for x in fileContent[s:e]), end=" ")
      if (comment is not None):
        print(comment)
      else:
        print()
    else:
      print(" ", end=" ")
      print(' '.join('0x{:02x},'.format(x) for x in fileContent[s:e]))
    s = e
  return

def skip_byte():
  print(f'  0x{fileContent[addr]:02x},')
  return

rType = fileContent[0]
dataFlag = fileContent[1]

if (dataFlag == 0 or dataFlag == 0xFF):
  # Acorn format
  indexStart = 64
  print("const uint8_t PHROMdata[] = {\n ", bout(0)+",", bout(1)+", // Format type, data flag")
  idx1 = reversed.find(0, 2)
  idx2 = reversed.find(0, idx1+1)
  idx3 = reversed.find(0, idx2+1)
  comment = ("// Copyright: "+reversed[2:idx1].decode('ascii')+" "+
             reversed[idx1+1:idx2].decode('ascii')+" "+reversed[idx2+1:idx3].decode('ascii'))
  dout(2,54,comment)
  serial = reversed[54]+reversed[55]*256
  dout(54,56,"// Serial: "+str(serial)+" (0 = Word PHROM)")
  nWords = reversed[56]+reversed[57]*256
  dout(56,58,"// No. of ASCII associated words: "+str(nWords))
  nPtrs = reversed[58]+reversed[59]*256
  dout(58,60,"// No. of pointers: "+str(nPtrs))
  pROMend = get_pointer(fileContent[60], fileContent[61])
  dout(60,62,"// Pointer to end of ROM data: "+hex(pROMend))
  pSpeechEnd = get_pointer(fileContent[62], fileContent[63])
  dout(62,64,"// Pointer to end of speech data: "+hex(pSpeechEnd))
  pEnd = fileContent[indexStart+(nPtrs)*2]
  if (pEnd != 255):
    raise Exception("Error at end of pointer table:", hex(pEnd))

  # Process the pointer table
  ptrs = []
  for w in range(nPtrs):
    a = indexStart+w*2
    b = indexStart+w*2+1
    pWord = get_pointer(fileContent[a], fileContent[b])
    dout(a,b+1, "// Phrase "+f'{w:03d}'+" @ "+f'0x{pWord:04x}')
    ptrs.append(pWord)
  print("  /*", f'0x{indexStart+(nPtrs)*2:04x}' ,"*/  0xff,", end=" ")
  ptrs_sorted = list(set(ptrs))
  ptrs_sorted.sort();
  ptrs_len = len(ptrs_sorted)
  print("// Unique phrases:", ptrs_len)

  # Process the phrase names
  phraseNameEnts = []
  for p in range(ptrs_len):
    s = ptrs_sorted[p]
    name = bytearray()
    # Read string backwards from the index entry
    while (validNameChar(reversed[s-1])):
      s -= 1;
      c = getNameChar(reversed[s]);
      name.extend(c);
    phraseNameEnts.append(phraseNameEnt(s, name));
    # print(f'{p:03d}', f'0x{s:04x}', name.decode('ascii'))

  # Process the phrase data
  for p in range(ptrs_len):
    if (p < ptrs_len-1):
      e = phraseNameEnts[p+1].pointer
    else:
      e = reversed.find(0, ptrs_sorted[p])
      if (e < 0):
        e = 0x4000
    s = phraseNameEnts[p].pointer
    # Text description (if any)
    print("  /*", f'0x{s:04x}', "*/", end="")
    dout(s,ptrs_sorted[p],"// "+phraseNameEnts[p].name.decode('ascii'))
    # Phrase data
    print("  /*", f'0x{ptrs_sorted[p]:04x}', "*/")
    dout(ptrs_sorted[p], e)
  print("  /*", f'0x{e:04x}', "*/")
  print('};')
  # List the phrases and their addresses
  #for p in range(ptrs_len):
  #  print(f'  {p:3d}', f'0x{phraseNameEnts[p].pointer:04x} ', phraseNameEnts[p].name.decode('ascii'))

elif (rType == 0x55):
  # TI-99 indexed format
  nPtrs = 373
  print("const uint8_t PHROMdata[] = {\n ", bout(0)+", // Type:", f'0x{rType:02x}')
  indexStart = 1
  ## Show phrase words and addresses
  ptrs = []
  ixPtr = indexStart
  prevPtr = indexStart
  for w in range(nPtrs):
    wl = reversed[ixPtr]
    ixPtr += 1
    word = reversed[ixPtr:ixPtr+wl].decode('ascii')
    ixPtr += wl
    data = reversed[ixPtr:ixPtr+8] # the purpose of these first 4 bytes is unknown
    if (data[4] != 0):
      raise Exception("Error parsing data, index byte 5 is not zero")
    ptr = get_pointer(data[6],data[5]) # Byte order reversed WRT indirect indexing
    phrLen = data[7]
    ptrs.append(phraseNameEnt(ptr, word, phrLen))
    ePtr = ixPtr + 8
    #print("  /*", f'0x{prevPtr:04x}', "*/", end="")
    dout(prevPtr,ePtr, "// Phrase "+f'{w:03d}: ' + word + f' -> 0x{ptr:04x}')
    prevPtr = ixPtr = ePtr
  ptrs_s = list(ptrs)
  ptrs_s.sort(key=ptrKey)
  ptrs_sorted = uniq(ptrs_s)
  ptrs_len = len(ptrs_sorted)
  print("  // Unique phrases:", ptrs_len)
  for p in range(ptrs_len):
    if (p < ptrs_len-1):
      e = ptrs_sorted[p+1].pointer
    else:
      e = reversed.find(0, ptrs_sorted[p].pointer)
      if (e < 0):
        e = 0x8000
    s = ptrs_sorted[p].pointer
    if (e-s != ptrs_sorted[p].phrLen):
      raise Exception("Error parsing data, length field differs", e-s, "vs", ptrs_sorted[p].phrLen)
    if (not validStartFrame(s)):
      raise Exception("Error parsing data, start frame is not valid", f'0x{s:04x}')
    ep = parsePhrase(s)
    print("  /*", f'0x{s:04x}', ptrs_sorted[p].name, "*/")
    if (e>s):
      dout(s, e, '// ->'+f'0x{ep:04x}')
  print("  /*", f'0x{e:04x}', "*/")
  print('};')

elif (rType != 0):
  # Unindexed PHROM
  print("const uint8_t PHROMdata[] = {")
  addr = 0
  count = 0
  while (addr < fileLen-1):
    if (addr > 0):
      # For some reason, each phrase is separated by a random? byte
      skip_byte()
      addr += 1
    # Check validity of the phrase start frame
    if (not validStartFrame(addr)):
      if (fileContent[addr] == 0):
        # End of data, dump the rest
        e = fileLen
      else:
        raise Exception("Error parsing data, start frame is not valid")
    else:
      e = parsePhrase(addr)
    if (e-addr > 1):
      print("  /*", f'0x{addr:04x}', "*/")
      dout(addr, e)
    else:
      print(f'  0x{fileContent[addr]:02x},')
    addr = e
    count += 1
  print('}; //', count, "phrases")

else:
  # TI indexed format, has two bytes followed by index and data
  nPtrs = dataFlag
  print("const uint8_t PHROMdata[] = {\n ", bout(0)+",", bout(1)+", // Format type, No. of phrases:", nPtrs)
  indexStart = 2
  # Show phrase addresses
  ptrs = []
  for w in range(nPtrs):
    a = indexStart+w*2
    b = indexStart+w*2+1
    pWord = get_pointer(fileContent[a], fileContent[b])
    dout(a,b+1, "// Phrase "+f'{w:03d}'+" @ "+f'0x{pWord:04x}')
    ptrs.append(pWord)
  # Sort the pointers
  ptrs_sorted = list(set(ptrs))
  ptrs_sorted.sort();
  ptrs_len = len(ptrs_sorted)
  # Show phrase data
  for p in range(len(ptrs_sorted)):
    if (p < len(ptrs_sorted)-1):
      e = ptrs_sorted[p+1]
    else:
      e = reversed.find(0, ptrs_sorted[p]) + 1
      if (e < 0):
        e = 0x4000
    print("  /*", f'0x{ptrs_sorted[p]:04x}', "*/")
    dout(ptrs_sorted[p], e)
  print("  /*", f'0x{e:04x}', "*/")
  print('};')
