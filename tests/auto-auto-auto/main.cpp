
#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QVariant>
#include <QtCore/qstringbuilder.h>

class MyString : public QString {
public:
  MyString(QString const &s = QString()) : QString(s) {}

  const QString *getConst() const { return this; }
  QString *const getConstOther() { return this; }
  const QString *const getConstConst() const { return this; }
  QString *get() { return this; }
  const QString getConstValue() const { return *this; }
};

MyString getMyString() { return MyString(); }

struct SomeStruct {
  int i1, i2;
  static int stint;
};

int SomeStruct::stint = 42;

void myfun(void *) {}

static const int num = 124;
static thread_local bool toggle = false;

int main(int argc, char **argv) {
  QCoreApplication app(argc, argv);

  QFile *f1 = new QFile(&app);
  QObject *f2 = new QFile(&app);
  QFile const *f3 = new QFile(&app);
  QObject const *f4 = new QFile(&app);
  QFile *f5 = qobject_cast<QFile *>(f2);
  QFile *f6 = dynamic_cast<QFile *>(f2);
  QFile *f7 = static_cast<QFile *>(f2);

  int i = 0;
  const int i2 = 0;
  bool b = false;

  int i3 = i;

  QString s = app.objectName();
  // QString::const_iterator is 'QChar const*'
  QString::const_iterator it = s.begin();
  QChar c1 = *it;
  const QChar c2 = *it;
  int sz = s.size();

  int combine = sz + i3;

  QVariant var = QVariant::fromValue(s);
  const QVariant var2 = QVariant::fromValue(s);
  QString s2 = var.value<QString>();

  QString s4 = QString::fromLatin1("foo");
  QString s5 = QStringLiteral("foo");

  MyString ms1 = getMyString();
  MyString ms2 = ms1;

  QString::const_iterator sit = s.begin(), send = s.end();
  QString *copy1 = ms1.get(), *copy2 = ms2.get();

  QString *cs3 = ms1.get();
  const QString *cs = ms1.getConst();
  QString *const cs6 = ms1.getConstOther();
  const QString *const cs4 = ms1.getConstConst();
  QString cqs = ms1.getConstValue();

  const QString *cs2 = ms1.get();
  const QString *const cs5 = ms1.getConst();
  const QString *const cs7 = ms1.getConstOther();

  for (QString::const_iterator it = s.begin(); it != s.end(); ++it) {
  }

  void (*fn)(void *) = myfun;
  void (*fn2)(void *) = fn;

  MyString ms3 = s4;
  QString s3 = QLatin1String("foo");
  unsigned int i4 = var.value<int>();
  QString s6 = getMyString();
  QString s7 = MyString();
  QString s8 = ms1;

  SomeStruct result = {4, 5};

  QString defaultInit;
  QFile uncopyable;
  QString multi = s3, another;
  int x = 0, y;
  int z, w = 0;
  unsigned int ui = 9;
  unsigned int r = ui, t = 0;

  QString copy21 = *ms1.get(), *copy22 = ms2.get();

  QString expr_result = copy21 % multi;

  try {
  } catch (QString s) {
  }

  return 0;
}
