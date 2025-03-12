use opendal::{services, Operator, Result};
use opendal::layers::LoggingLayer;

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
    let mut w = op.writer_with(path).append(true).await?;
    w.write(data.clone()).await?;
    w.write(data.clone()).await?;
    w.close().await?;
    
    // Read
    match op.read(path).await {
        Ok(data) => {
            println!("Read from path ({}): {}", path, String::from_utf8_lossy(&data.to_vec()));

        }
        Err(e) => {
            println!("Failed to read from path ({}): {:?}", path, e);
            return Err(e)
        }
    }

    // Write again
    let mut w = op.writer_with(path).append(true).await?;
    w.write("Another write".as_bytes().to_vec()).await?;
    w.close().await?;

    Ok(())
}
