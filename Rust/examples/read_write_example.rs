use opendal::Result;
use opendal::layers::LoggingLayer;
use opendal::services;
use opendal::Operator;
use opendal::Buffer;

// Path Constants
const TEST_PATH: &str = "/testpath";
const TEST_PATH_SECOND: &str = "/testpath/secondpath";
const TEST_PATH_NESTED: &str = "/testpath/testpath";

fn create_bytes(data: &str) -> Vec<u8> {
    data.as_bytes().to_vec()
}

fn format_bytes(data: &[u8]) -> String {
    String::from_utf8_lossy(data).to_string()
}

fn format_buffer(buffer: &Buffer) -> String {
    let bytes: Vec<u8> = buffer.to_bytes().to_vec();
    String::from_utf8_lossy(&bytes).to_string()
}

async fn write_with_logging(op: &Operator, path: &str, data: &[u8]) -> Result<()> {
    let data_owned = data.to_vec();
    match op.write(path, data_owned).await {
        Ok(_) => {
            println!("Wrote to path ({}): {}", path, format_bytes(data));
            Ok(())
        }
        Err(e) => {
            println!("Failed to write to path ({}): {:?}", path, e);
            Err(e)
        }
    }
}

async fn read_with_logging(op: &Operator, path: &str) -> Result<Buffer> {
    match op.read(path).await {
        Ok(data) => {
            println!("Read from path ({}): {}", path, format_buffer(&data));
            Ok(data)
        }
        Err(e) => {
            println!("Failed to read from path ({}): {:?}", path, e);
            Err(e)
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////

async fn test1(op: &Operator, path: &str) -> Result<()> {
    let result = op.read(path).await;
    match result {
        Ok(_) => {
            println!("Expected an error, but got data");
            panic!("Expected an error when reading non-existent path");
        }
        Err(e) => {
            println!("Received expected error: {:?}", e);
            Ok(())
        }
    }
}

async fn test2(op: &Operator, path: &str, data: &[u8]) -> Result<()> {
    write_with_logging(op, path, data).await?;
    read_with_logging(op, path).await?;
    Ok(())
}

async fn test3(op: &Operator, path: &str, small_data: &[u8], big_data: &[u8]) -> Result<()> {
    // Write bigger data
    write_with_logging(op, path, big_data).await?;
    // Write smaller data
    write_with_logging(op, path, small_data).await?;
    // Read the result
    read_with_logging(op, path).await?;
    Ok(())
}

async fn test4(op: &Operator, path1: &str, path2: &str, path3: &str, data: &[u8], bigger_data: &[u8], special_data: &[u8]) -> Result<()> {
    // Write to paths
    write_with_logging(op, path1, data).await?;
    write_with_logging(op, path2, bigger_data).await?;
    write_with_logging(op, path3, special_data).await?;
    // Read from paths
    read_with_logging(op, path1).await?;
    read_with_logging(op, path2).await?;
    read_with_logging(op, path3).await?;
    Ok(())
}

///////////////////////////////////////////////////////////////////////////////////////////

#[tokio::main]
async fn main() -> Result<()> {
    let builder = services::Memory::default();

    let op = Operator::new(builder)?
        .layer(LoggingLayer::default())
        .finish();

    let data = create_bytes("Test message");
    let bigger_data = create_bytes("A bigger text with more characters");
    let special_data = create_bytes("Text with some special characters: !@#$%^&*()_+☺");

    println!("\n------------ Test1: read non-existent path ---------------------\n");
    test1(&op, TEST_PATH).await?;
    println!("\n------------ Test2: write and read -----------------------------\n");
    test2(&op, TEST_PATH, &data).await?;
    println!("\n------------ Test3: double write -------------------------------\n");
    test3(&op, TEST_PATH, &data, &bigger_data).await?;
    println!("\n------------ Test4: chained paths and special chars ------------\n");
    test4(&op, TEST_PATH,TEST_PATH_SECOND, TEST_PATH_NESTED, &data, &bigger_data, &special_data).await?;
    println!("\n----------------------------------------------------------------\n");
    Ok(())
}
