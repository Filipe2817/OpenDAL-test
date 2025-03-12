use opendal::{services, Error, ErrorKind, Operator, Reader, Result};
use opendal::layers::LoggingLayer;
use futures::{io::AsyncReadExt, stream::TryStreamExt};

async fn test_direct_read(reader: &Reader, len: u64) -> Result<()> {
    let buffer = reader.read(0..len).await?;
    println!("\nDirect read: {}\n", String::from_utf8_lossy(&buffer.to_bytes()));
    Ok(())
}

async fn test_stream_read(reader: &Reader, len: u64) -> Result<()> {
    let stream = reader.clone().into_bytes_stream(0..len).await?;
    
    let collected_chunks = stream
        .try_collect::<Vec<_>>()
        .await
        .map_err(|e| Error::new(ErrorKind::Unexpected, e.to_string()))?;
    
    let stream_data: Vec<u8> = collected_chunks.iter().flatten().cloned().collect();
    println!("\nStream read: {}\n", String::from_utf8_lossy(&stream_data));
    Ok(())
}

async fn test_async_read(reader: &Reader, len: u64) -> Result<()> {
    let mut async_reader = reader.clone().into_futures_async_read(0..len).await?;
    let mut async_data = Vec::new();

    async_reader
        .read_to_end(&mut async_data)
        .await
        .map_err(|e| Error::new(ErrorKind::Unexpected, e.to_string()))?;
    
    println!("\nAsyncRead read: {}\n", String::from_utf8_lossy(&async_data));
    Ok(())
}

async fn test_concurrent_read(op: &Operator, path: &str, len: u64) -> Result<()> {
    let mut concurrent_data = Vec::new();
    let mut concurrent_reader = op
        .reader_with(path)
        .concurrent(8)
        .chunk(32)
        .await?
        .into_futures_async_read(0..len)
        .await?;

    concurrent_reader
        .read_to_end(&mut concurrent_data)
        .await
        .map_err(|e| Error::new(ErrorKind::Unexpected, e.to_string()))?;
    
    println!("\nConcurrent AsyncRead read: {}\n", String::from_utf8_lossy(&concurrent_data));
    Ok(())
}

///////////////////////////////////////////////////////////////////////////////////////////

#[tokio::main]
async fn main() -> Result<()> {
    let builder = services::Memory::default();
    let op = Operator::new(builder)?
        .layer(LoggingLayer::default())
        .finish();

    let path = "/testpath";
    let data = "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book."
        .as_bytes()
        .to_vec();

    // Write
    match op.write(path, data.clone()).await {
        Ok(_) => {
            println!("\nWrote to path ({}): {}\n", path, String::from_utf8_lossy(&data).to_string());
        }
        Err(e) => {
            println!("Failed to write to path ({}): {:?}", path, e);
            return Err(e);
        }
    }

    let reader = op.reader(path).await?;
    let data_len = data.len() as u64;

    // Test different reader methods
    test_direct_read(&reader, data_len).await?;
    test_stream_read(&reader, data_len).await?;
    test_async_read(&reader, data_len).await?;

    // Test reader_with method
    test_concurrent_read(&op, path, data_len).await?;
    Ok(())
}
