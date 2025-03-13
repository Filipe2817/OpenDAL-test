use opendal::{services, Operator, Result, Buffer};
use opendal::layers::LoggingLayer;

async fn read_with_logging(op: &Operator, path: &str) -> Result<Buffer> {
    match op.read(path).await {
        Ok(res) => {
            println!("Read from path ({}): {}", path, String::from_utf8_lossy(&res.to_vec()));
            Ok(res)
        }
        Err(e) => {
            println!("Failed to read from path ({}): {:?}", path, e);
            Err(e)
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////

#[tokio::main]
async fn main() -> Result<()> {
    let builder = services::Fs::default().root("/tmp/opendal");
    let op = Operator::new(builder)?
        .layer(LoggingLayer::default())
        .finish();

    let path = "/testpath";
    let data = "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book."
        .as_bytes()
        .to_vec();

    // Write
    let mut w = op.writer_with(path).await?;
    w.write(data.clone()).await?;

    // Read before closing
    read_with_logging(&op, path).await?;

    w.close().await?;
    
    // Read after closing
    read_with_logging(&op, path).await?;

    // Write again with multiple options
    let mut w = op
        .writer_with(path)
        .append(true)
        .chunk(128)
        .concurrent(2)
        .await?;
    w.write(data[..data.len() / 2].to_vec()).await?;
    w.write(data[data.len() / 2..].to_vec()).await?;
    w.close().await?;

    read_with_logging(&op, path).await?;

    Ok(())
}
