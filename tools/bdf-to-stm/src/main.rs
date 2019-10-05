use bdf;
use itertools::Itertools;
use std::{env, error::Error, io, iter};

const CHARSET: &[char] = &[
    ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', '0', '1', '2',
    '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', '@', 'A', 'B', 'C', 'D', 'E',
    'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', '[', '\\', ']', '^', '_', '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k',
    'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~',
];

fn main() -> Result<(), Box<dyn Error>> {
    let name = env::args().nth(1).expect("no font name provided");
    let font = bdf::read(io::stdin())?;

    println!("#include \"fonts.h\"");
    println!("const uint8_t {name}_Table[] =", name = name);
    println!("{{");

    for (&ch, glyph) in CHARSET
        .iter()
        .map(|ch| (ch, font.glyphs().get(ch).unwrap()))
    {
        println!();
        println!(
            "\t// @{codepoint} {ch:?} ({width} pixels wide)",
            codepoint = ch as usize,
            ch = ch,
            width = glyph.width()
        );

        for y in 0..glyph.height() {
            print!("\t");

            let mut draw = String::new();

            for bits in &iter::repeat(false)
                .take(glyph.bounds().x as usize)
                .chain((0..glyph.width()).map(|x| glyph.map().get(x, y)))
                .chunks(8)
            {
                let mut bits = bits.collect::<Vec<_>>();
                if bits.len() < 8 {
                    for _ in 0..8 - bits.len() {
                        bits.push(false);
                    }
                }

                let byte = bits
                    .into_iter()
                    .rev()
                    .enumerate()
                    .fold(0, |acc, (i, b)| acc | ((b as u8) << i));

                draw.push_str(&format!("{1:00$b}", glyph.width() as usize, byte));

                print!("0x{:02X}, ", byte);
            }

            println!("//{}", draw.replace("1", "#").replace("0", " "));
        }

        // Print the right amount of empty lines.
        for _ in 0..font.bounds().y.abs() + glyph.bounds().y {
            for _ in 0..(glyph.width() + glyph.bounds().x as u32) / 8 {
                print!("0x{:02X}, ", 0);
            }

            println!();
        }
    }

    println!("}};");
    println!();

    println!(
        "sFONT {name} = {{ {name}_Table, {width}, {height} }};",
        name = name,
        width = font.bounds().width,
        height = font.bounds().height
    );

    Ok(())
}
