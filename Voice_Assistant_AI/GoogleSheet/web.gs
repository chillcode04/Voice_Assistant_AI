// Code.gs
function doGet() {
  return HtmlService.createHtmlOutputFromFile('form');  // dùng tên file form.html
}

function saveConfig(mssv, dateStr) {
  var props = PropertiesService.getScriptProperties();
  props.setProperty('CURRENT_MSSV', mssv);
  props.setProperty('CURRENT_DATE', dateStr); // ví dụ: "18/05/2025"
}


function doPost(e) {
  const sheet = SpreadsheetApp.getActiveSpreadsheet().getSheetByName("Sheet2");
  const data = JSON.parse(e.postData.contents);
  const content = data.content;

  const props = PropertiesService.getScriptProperties();
  const mssv = props.getProperty('CURRENT_MSSV');
  const dateStr = props.getProperty('CURRENT_DATE'); // dạng "dd/MM/yyyy"

  if (!mssv || !dateStr) {
    return ContentService.createTextOutput("Vui lòng nhập MSSV và ngày trước trên giao diện web.");
  }

  const allData = sheet.getDataRange().getValues();

  // Tìm hàng chứa MSSV ở cột E
  let row = -1;
  for (let i = 0; i < allData.length; i++) {
    if (allData[i][4] && allData[i][4].toString().trim() === mssv.trim()) {
      row = i + 1;
      break;
    }
  }
  if (row === -1) {
    return ContentService.createTextOutput("Không tìm thấy MSSV: " + mssv);
  }

  // Tìm cột chứa ngày trong hàng 1
  const headerRow = allData[0];
  let col = -1;
  for (let j = 0; j < headerRow.length; j++) {
    let cellDate = new Date(headerRow[j]);
    let formatted = Utilities.formatDate(cellDate, "Asia/Ho_Chi_Minh", "dd/MM/yyyy");
    if (formatted === dateStr) {
      col = j + 1;
      break;
    }
  }
  if (col === -1) {
    return ContentService.createTextOutput("Không tìm thấy ngày: " + dateStr + " trong hàng tiêu đề.");
  }

  // Ghi nội dung
  sheet.getRange(row, col).setValue(content);
  return ContentService.createTextOutput(`Ghi thành công vào MSSV ${mssv}, ngày ${dateStr}`);
}
