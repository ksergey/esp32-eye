import sys
from PIL import Image

def rgb_hex565(red, green, blue):
    u16 = (int(red / 255 * 31) << 11) | (int(green / 255 * 63) << 5) | (int(blue / 255 * 31))
    return '0x{:04x}'.format(u16)
    # return '0x{:02x} 0x{:02x}'.format(int(u16) >> 8, int(u16) & 0xFF)

def main():
    width = 240
    height = 240
    with Image.open(sys.argv[1]) as im:
        print(f'#include <cstdint>')
        print(f'#include <cstring>')
        print('')
        print(f'constexpr std::size_t frames_count = {im.n_frames};')
        print(f'constexpr std::size_t frame_width = {width};')
        print(f'constexpr std::size_t frame_height = {height};')
        print('')
        print('struct gif_frame {')
        print(f'  unsigned duration;')
        print(f'  std::uint16_t pixels[{ width * height }];')
        print('};')
        print('')
        print(f'static gif_frame frames[{im.n_frames}] = {{');

        for i in range(im.n_frames):
            im.seek(i)

            duration = im.info['duration'] # ms

            rgb_im = im.convert('RGB')
            rgb_im.resize((width, height))

            content = ''
            for x in range(width):
                for y in range(height):
                    (r, g, b) = rgb_im.getpixel((x, y))
                    content += rgb_hex565(r, g, b) + ' '

            print('  {')
            print(f'    {duration},')
            print('    {')
            for l in zip(*(iter(content.split()),) * 8):
                print(f'      {', '.join(l)},')
            print('    }')
            print('  },')

        print(f'}};');


if __name__ == '__main__':
    main()
