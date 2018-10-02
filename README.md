# ykcmp
A decompressor & compressor for the YKCMP_V1 format, used extensively in Nippon Ichi Software games.
Decompression code is mostly adapted from [XKeeper0's gist](https://gist.github.com/Xkeeper0/d1ef62e5464e8bbfa655b556a78af1ac);
compression is homemade.

## Usage
### Decompression
Basic usage:
`ykcmp.exe input_file [output_file]`

If not specified, outputs to input_file.dec.
If the YKCMP archive is located somewhere other than at the beginning of the file, you can use `-a` to specify its offset:
`ykcmp.exe -a 0x80 input_file [output_file]`.
The size of the archive will be identified automatically.

### Compression
Basic usage:
`ykcmp.exe -c input_file [output_file]`

If not specified, outputs to input_file.ykcmp. You can also specify the level of compression (0-2) using the `-l` option, e.g.:
`ykcmp.exe -cl 1 input_file [output_file]`. The default level is 1.

#### Compression levels
0 = No compression. Use this if you just need a YKCMP archive and the file size doesn't matter.
Really fast, but inflates the file size by ~0.79%.

1 = Decent compression. Several times faster than level 2; also outperforms level 2 in terms of file size on files with little repetition.
On other files the difference in size between levels 1 and 2 is negligible, so level 1 is recommended.

2 = Best compression. Works better if the file has a lot of repetition. It's also very slow, so likely not worth it.

## A note about the format
The header of the archive contains the archive's size at offset 0xC (let's call it zsize).

The code by XKeeper0, which this project is partially based on, seems to assume that zsize refers not to the size of the whole archive,
but only to the size of compressed data (i.e. it excludes the header).

However – although I only looked at two different games – it seems that in reality zsize refers to the size of the whole archive,
including the header. This program also operates under this assumption. If the assumption is ever shown to be wrong, I'll change it.
