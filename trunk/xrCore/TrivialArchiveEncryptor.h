#pragma once

class TrivialArchiveEncryptor
{
private:
	class Random32
	{
	private:
		u32 m_Seed;

	public:
		Random32(const u32& seed) : m_Seed(seed) {}

		IC u32 random(const u32& range)
		{
			m_Seed = 0x08088405 * m_Seed + 1;
			const u64 temp = static_cast<u64>(m_Seed) * static_cast<u64>(range);
			return static_cast<u32>(temp >> 32);
		}
	};

public:
	enum class Format
	{
		Russian,
		WorldWide
	};

private:
	struct ArchiveFormat
	{
		u32 m_table_iterations;
		u32 m_table_seed;
		u32 m_encrypt_seed;
	};

	static const inline ArchiveFormat FormatRussian { 2048, 20091958, 20031955 };
	static const inline ArchiveFormat FormatWorldWide { 1024, 6011979, 24031979 };

	static const ArchiveFormat* GetFormat(Format format)
	{
		switch (format)
		{
		case Format::Russian:
			return &FormatRussian;

		case Format::WorldWide:
			return &FormatWorldWide;
		}

		return nullptr;
	}

	static constexpr u32 AlphabetSize = static_cast<u32>(1 << (8 * sizeof(u8)));
	static inline u8 m_alphabet_back[AlphabetSize];
	static inline u8 m_alphabet[AlphabetSize];

	static const inline ArchiveFormat* m_CurrentFormat = nullptr;

	IC static void Initialize(Format format)
	{
		auto NewFormat = GetFormat(format);

		if (m_CurrentFormat == NewFormat)
			return;

		m_CurrentFormat = NewFormat;
		
		for (u32 i = 0; i < AlphabetSize; ++i)
			m_alphabet[i] = static_cast<u8>(i);

		Random32 rand(m_CurrentFormat->m_table_seed);

		for (u32 i = 0; i < m_CurrentFormat->m_table_iterations; ++i)
		{
			u32 j = rand.random(AlphabetSize);
			u32 k = rand.random(AlphabetSize);

			while (j == k)
				k = rand.random(AlphabetSize);

			std::swap(m_alphabet[j], m_alphabet[k]);
		}

		for (u32 i = 0; i < AlphabetSize; ++i)
			m_alphabet_back[m_alphabet[i]] = static_cast<u8>(i);
	}

public:
	IC static void Encode(const void* source, const u32& sourceSize, void* destination)
	{
		Random32 rand(m_CurrentFormat->m_encrypt_seed);
		const u8* I = (const u8*)source;
		const u8* E = (const u8*)source + sourceSize;
		u8* J = (u8*)destination;
		for (; I != E; ++I, ++J)
			*J = m_alphabet[*I] ^ static_cast<u8>(rand.random(256) & 0xff);
	}

	IC static void Decode(const void* source, const u32& sourceSize, void* destination, Format format)
	{
		Initialize(format);
		
		Random32 rand(m_CurrentFormat->m_encrypt_seed);
		const u8* I = (const u8*)source;
		const u8* E = (const u8*)source + sourceSize;
		u8* J = (u8*)destination;

		for (; I != E; ++I, ++J)
			*J = m_alphabet_back[(*I) ^ static_cast<u8>(rand.random(256) & 0xff)];
	}
};

extern TrivialArchiveEncryptor TrivialLocatorEncryptor;