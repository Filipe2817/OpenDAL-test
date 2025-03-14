use opendal::Result;
use opendal::layers::LoggingLayer;
use opendal::services;
use opendal::Operator;


#[tokio::main]
async fn main() -> Result<()> {
    // Pick a builder and configure it.
    let builder = services::Memory::default();

    // Init an operator
    let op = Operator::new(builder)?
        // Init with logging layer enabled.
        .layer(LoggingLayer::default())
        .finish();

    // Write data
    op.write("hello.txt", "Hello, World!").await?;

    // Read data
    let bs = op.read("hello.txt").await?;
    println!("Read content: {:?}", bs);

    // Fetch metadata
    let meta = op.stat("hello.txt").await?;
    let mode = meta.mode();
    let length = meta.content_length();
    println!("File mode: {:?}", mode);
    println!("Content length: {}", length);

    // Delete
    op.delete("hello.txt").await?;

    Ok(())
}
