import fs from "fs";
import http from "http";
import { dirname } from "path";
import { fileURLToPath } from "url";
import "dotenv/config";
import { GoogleGenerativeAI } from "@google/generative-ai";
import { GoogleAIFileManager, FileState } from "@google/generative-ai/server";

const __dirname = dirname(fileURLToPath(import.meta.url));
const TOTAL_PARTS = 4;

const partStatus = Array(TOTAL_PARTS).fill(false); // Theo dõi phần nào đã nhận

const server = http.createServer();
server.maxConnections = 100; 
server.on("request", async (request, response) => {
  if (request.method === "POST" && request.url === "/uploadAudio") {
    const taskIndex = request.headers["x-task-index"];
    if (taskIndex === undefined) {
      response.writeHead(400);
      response.end("Missing X-Task-Index header");
      return;
    }

    const partPath = `${__dirname}/recording_part_${taskIndex}.wav`;
    const recordingFile = fs.createWriteStream(partPath, { encoding: null });

    request.on("data", (chunk) => recordingFile.write(chunk));
    request.on("end", async () => {
      recordingFile.end();
      partStatus[taskIndex] = true;
      console.log(`Received part ${taskIndex}`);

      // Check nếu đã đủ tất cả các phần
      if (partStatus.every((s) => s)) {
        console.log("Received all parts. Merging...");
        await mergeWavFiles(TOTAL_PARTS, `${__dirname}/recording.wav`);
        console.log("Merged successfully. Uploading...");

        try {
          const result = await HandleFile(`${__dirname}/recording.wav`);
          response.writeHead(200, { "Content-Type": "text/plain" });
          response.end(result);

          partStatus = Array(TOTAL_PARTS).fill(false);
          for (let i = 0; i < TOTAL_PARTS; i++) {
            fs.unlinkSync(`${__dirname}/recording_part_${i}.wav`);
          }
        } catch (err) {
          console.error("Gemini processing error:", err);
          response.writeHead(500, { "Content-Type": "text/plain" });
          response.end("Error processing audio");
        }
      } 
    });
  } else {
    console.log("Error: Check your POST request");
    response.writeHead(405, { "Content-Type": "text/plain" });
    response.end("Method Not Allowed");
  }
});

async function mergeWavFiles(totalParts, outputPath) {
  const output = fs.createWriteStream(outputPath);
  // const header = fs.readFileSync(`${__dirname}/recording_part_0.wav`, { encoding: null, flag: "r" }).subarray(0, 44); // chỉ lấy header từ part 0
  // output.write(header);

  for (let i = 0; i < totalParts; i++) {
    const data = fs.readFileSync(`${__dirname}/recording_part_${i}.wav`);
    //const body = i === 0 ? data.subarray(44) : data // bỏ header của Part0
    output.write(data);
  }

  output.end();
}

async function HandleFile(mediaPath) {
  const fileManager = new GoogleAIFileManager(process.env.GEMINI_API_KEY);

  const uploadResult = await fileManager.uploadFile(mediaPath, {
    mimeType: "audio/wav",
    displayName: "Audio record",
  });

  let file = await fileManager.getFile(uploadResult.file.name);
  while (file.state === FileState.PROCESSING) {
    process.stdout.write(".");
    // Sleep for 10 seconds
    await new Promise((resolve) => setTimeout(resolve, 5_000));
    // Fetch the file from the API again
    file = await fileManager.getFile(uploadResult.file.name);
  }

  if (file.state === FileState.FAILED) {
    throw new Error("Audio processing failed.");
  }

  // View the response.
  console.log(
    `Uploaded file ${uploadResult.file.displayName} as: ${uploadResult.file.uri}`,
  );

  const genAI = new GoogleGenerativeAI(process.env.GEMINI_API_KEY);
  const model = genAI.getGenerativeModel({ model: "gemini-1.5-flash" });
  const generationConfig = {
  temperature: 1.5,
  topP: 0.95,
  topK: 64,
  maxOutputTokens: 8192,
  responseModalities: [
  ],
  responseMimeType: "text/plain",
}
  const result = await model.generateContent([
    
  `  Lưu ý: Ghi cực kỳ ngắn gọn, súc tích và dễ hiểu.
  Đây là đoạn audio ghi lại phần báo cáo đồ án giữa sinh viên và giảng viên.
   Bạn hãy nghe kỹ và phân tích nội dung báo cáo theo 2 phần sau
  - Công việc đã làm trong 2 tuần vừa qua (ghi thành các ý gạch đầu dòng)
  - Kế hoạch thực hiện trong 2 tuần tới (ghi thành các ý gạch đầu dòng)
  Lưu ý: chỉ ghi 2 nội dung trên ngoài ra không ghi thêm bất cứ nội dung nào, và tuân thủ tiêu chí, ngắn gọn, cô đọng xúc tích.`,
    {
      fileData: {
        fileUri: uploadResult.file.uri,
        mimeType: uploadResult.file.mimeType,
      },
    },
  ]);
  console.log(result.response.text());
  return result.response.text();  
}

const port = 8888;
server.listen(port, () => {
  console.log(`Listening at ${port}`);
});


