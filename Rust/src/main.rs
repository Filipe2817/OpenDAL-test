use opendal::Result;
use opendal::layers::{LoggingLayer, FastraceLayer};
use opendal::services;
use opendal::Operator;

mod caesar_layer;
use caesar_layer::CaesarCipherLayer;

#[tokio::main]
async fn main() -> Result<()> {
    let builder = services::Memory::default();

    let op = Operator::new(builder)?
        .layer(LoggingLayer::default())
        .layer(CaesarCipherLayer::new(3))
        .layer(FastraceLayer)
        .finish();

    op.write("hello.txt", "Hello, World!").await?;

    let bs = op.read("hello.txt").await?;
    println!("Read content: {:?}", bs);

    let meta = op.stat("hello.txt").await?;
    let mode = meta.mode();
    let length = meta.content_length();
    println!("File mode: {:?}", mode);
    println!("Content length: {}", length);

    op.delete("hello.txt").await?;
    Ok(())
}
