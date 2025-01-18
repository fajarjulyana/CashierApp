#include <wx/wx.h>
#include <sqlite3.h>
#include <string>
#include <vector>

// Deklarasi KasirFrame
class KasirFrame : public wxFrame {
public:
    KasirFrame(const wxString& title);
    ~KasirFrame();

private:
    sqlite3* db;

    // Komponen UI
    wxTextCtrl* txtID;
    wxTextCtrl* txtNama;
    wxTextCtrl* txtHarga;
    wxTextCtrl* txtJumlah;
    wxTextCtrl* txtSearch;  // Untuk pencarian
    wxListBox* lstBarang;

    // Fungsi
    void OnTambahBarang(wxCommandEvent& event);
    void OnHapusBarang(wxCommandEvent& event);
    void OnSearchBarang(wxCommandEvent& event);  // Fungsi untuk pencarian
    void LoadBarang(const std::string& searchQuery = "");  // Fungsi untuk memuat barang

    wxDECLARE_EVENT_TABLE();
};

// ID untuk tombol
enum {
    ID_TambahBarang = 1,
    ID_HapusBarang,
    ID_SearchBarang  // ID untuk pencarian
};

// Event Table
wxBEGIN_EVENT_TABLE(KasirFrame, wxFrame)
    EVT_BUTTON(ID_TambahBarang, KasirFrame::OnTambahBarang)
    EVT_BUTTON(ID_HapusBarang, KasirFrame::OnHapusBarang)
    EVT_BUTTON(ID_SearchBarang, KasirFrame::OnSearchBarang)  // Event pencarian
wxEND_EVENT_TABLE()

// Implementasi Konstruktor KasirFrame
KasirFrame::KasirFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(600, 400)) {
    // Inisialisasi SQLite
    if (sqlite3_open("kasir.db", &db)) {
        wxMessageBox("Gagal membuka database!", "Error", wxICON_ERROR);
        Close();
    }


    // Layout UI
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    auto* formSizer = new wxGridSizer(2, 5, 5);

    formSizer->Add(new wxStaticText(this, wxID_ANY, "ID Barang:"), 0, wxALL, 5);
    txtID = new wxTextCtrl(this, wxID_ANY);
    formSizer->Add(txtID, 0, wxEXPAND | wxALL, 5);

    formSizer->Add(new wxStaticText(this, wxID_ANY, "Nama Barang:"), 0, wxALL, 5);
    txtNama = new wxTextCtrl(this, wxID_ANY);
    formSizer->Add(txtNama, 0, wxEXPAND | wxALL, 5);

    formSizer->Add(new wxStaticText(this, wxID_ANY, "Harga:"), 0, wxALL, 5);
    txtHarga = new wxTextCtrl(this, wxID_ANY);
    formSizer->Add(txtHarga, 0, wxEXPAND | wxALL, 5);

    formSizer->Add(new wxStaticText(this, wxID_ANY, "Jumlah:"), 0, wxALL, 5);
    txtJumlah = new wxTextCtrl(this, wxID_ANY);
    formSizer->Add(txtJumlah, 0, wxEXPAND | wxALL, 5);

    mainSizer->Add(formSizer, 0, wxEXPAND | wxALL, 10);

    // Pencarian
    auto* searchSizer = new wxBoxSizer(wxHORIZONTAL);
    txtSearch = new wxTextCtrl(this, wxID_ANY);
    searchSizer->Add(new wxStaticText(this, wxID_ANY, "Cari Barang:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    searchSizer->Add(txtSearch, 1, wxEXPAND | wxALL, 5);
    searchSizer->Add(new wxButton(this, ID_SearchBarang, "Cari"), 0, wxALL, 5);

    mainSizer->Add(searchSizer, 0, wxEXPAND | wxALL, 10);

    auto* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    btnSizer->Add(new wxButton(this, ID_TambahBarang, "Tambah Barang"), 0, wxALL, 5);
    btnSizer->Add(new wxButton(this, ID_HapusBarang, "Hapus Barang"), 0, wxALL, 5);

    mainSizer->Add(btnSizer, 0, wxALIGN_CENTER);

    lstBarang = new wxListBox(this, wxID_ANY);
    mainSizer->Add(lstBarang, 1, wxEXPAND | wxALL, 10);

    SetSizer(mainSizer);

    LoadBarang();  // Memuat semua barang saat aplikasi dijalankan
}

// Destruktor
KasirFrame::~KasirFrame() {
    sqlite3_close(db);
}

// Load Barang dengan Pencarian
void KasirFrame::LoadBarang(const std::string& searchQuery) {
    lstBarang->Clear();

    // Modifikasi query untuk pencarian
    std::string query = "SELECT id, nama, harga, jumlah FROM barang";
    if (!searchQuery.empty()) {
        query += " WHERE nama LIKE '%" + searchQuery + "%'";  // Pencarian berdasarkan nama
    }

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        wxMessageBox("Gagal memuat data barang!", "Error", wxICON_ERROR);
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string nama = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        double harga = sqlite3_column_double(stmt, 2);
        int jumlah = sqlite3_column_int(stmt, 3);

        lstBarang->Append(wxString::Format("%s - %s - Rp%.2f - %d", id, nama, harga, jumlah));
    }

    sqlite3_finalize(stmt);
}

// Tambah Barang
void KasirFrame::OnTambahBarang(wxCommandEvent&) {
    std::string id = txtID->GetValue().ToStdString();
    std::string nama = txtNama->GetValue().ToStdString();
    double harga = std::stod(txtHarga->GetValue().ToStdString());
    int jumlah = std::stoi(txtJumlah->GetValue().ToStdString());

    const char* insertSQL = "INSERT INTO barang (id, nama, harga, jumlah) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, nama.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 3, harga);
        sqlite3_bind_int(stmt, 4, jumlah);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            wxMessageBox("Gagal menambahkan barang!", "Error", wxICON_ERROR);
        } else {
            LoadBarang();  // Refresh daftar barang setelah menambah
        }
    }
    sqlite3_finalize(stmt);
}

// Hapus Barang
void KasirFrame::OnHapusBarang(wxCommandEvent&) {
    int sel = lstBarang->GetSelection();
    if (sel == wxNOT_FOUND) {
        wxMessageBox("Pilih barang yang ingin dihapus!", "Info", wxICON_INFORMATION);
        return;
    }

    std::string selected = lstBarang->GetString(sel).ToStdString();
    std::string id = selected.substr(0, selected.find(" - "));

    const char* deleteSQL = "DELETE FROM barang WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, deleteSQL, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            wxMessageBox("Gagal menghapus barang!", "Error", wxICON_ERROR);
        } else {
            LoadBarang();  // Refresh daftar barang setelah menghapus
        }
    }
    sqlite3_finalize(stmt);
}

// Fungsi Pencarian
void KasirFrame::OnSearchBarang(wxCommandEvent&) {
    std::string searchQuery = txtSearch->GetValue().ToStdString();
    LoadBarang(searchQuery);  // Memuat barang sesuai dengan pencarian
}

// Main App
class KasirApp : public wxApp {
public:
    virtual bool OnInit();
};

wxIMPLEMENT_APP(KasirApp);

bool KasirApp::OnInit() {
    KasirFrame* frame = new KasirFrame("Aplikasi Kasir - CRUD Barang");
    frame->Show(true);
    return true;
}

